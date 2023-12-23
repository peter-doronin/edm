/*
 * process.c
 *
 *  Created on: 18 нояб. 2023 г.
 *      Author: user
 */
#include "process.h"
#include "utils.h"
#include "stepgen.h"
#include "liquidcrystal_i2c.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim4;
extern ADC_HandleTypeDef hadc1;

process_data_t p_data = {0};
sg_traj_t traj = {0};
sg_tim_t sg_tim = {0};
sg_tim_t sg_tim2 = {0};
sw_encoder_t enc = {0};
char disp_buf1[17];
char disp_buf2[17];
uint16_t adc[3] = {0};

void process_loop(void){
	static bool init;
	if(!init){
		init = 1;
		enc.port_A = GPIOB;
		enc.pin_A = GPIO_PIN_4;
		enc.port_B = GPIOB;
		enc.pin_B = GPIO_PIN_3;

		p_data.vel_jog = 10;
		p_data.speed_rot = 1;
		p_data.state = STATE_JOG;

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_NoCursor();
		HD44780_Display();
	}

	//GPIO POLLING
	static bool bt1_prev; //bt1
	bool bt1 = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
	bool bt1_rise = 0;
	if(bt1 > bt1_prev) { 	//button push action
		bt1_rise = 1;
	}
	bt1_prev = bt1;


	static bool bt2_prev; //bt2
	bool bt2 = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);
	bool bt2_rise = 0;
	if(bt2 > bt2_prev) {	//button push action
		bt2_rise = 1;
	}
	bt2_prev = bt2;

	static bool bt3_prev; //bt3
	bool bt3 = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15);
	if(bt3 > bt3_prev) {
		p_data.is_auto ^= 1;
	}
	bt3_prev = bt3;

	static bool bt4_prev; //bt4
	bool bt4 = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
	if(bt4 > bt4_prev) {
		p_data.disp_view++;
		p_data.disp_view %= 3;
	}
	bt4_prev = bt4;

	static bool enc_push_prev; //encoder
	bool enc_push = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5);
	if(enc_push > enc_push_prev) {
		p_data.var_change_sel++;
		p_data.var_change_sel %= 8;
	};
	enc_push_prev = enc_push; //encoder
	//GPIO POLLING/

	if(!p_data.is_auto) p_data.state = STATE_JOG;
	if(p_data.curr_fb > 1.0f) p_data.is_short = 1;
	char lcd_state = 0;
	switch(p_data.state){
	case STATE_JOG:
		lcd_state = 'J';
		if(p_data.is_auto) p_data.state = STATE_AUTO_IDLE;
		traj.vel_mode = 1;
		traj.vel_cmd = 0;
		if(bt1) traj.vel_cmd = p_data.vel_jog;
		if(bt2) traj.vel_cmd = -p_data.vel_jog;
		break;
	case STATE_SEARCH:
		lcd_state = 'S';
		traj.pos_cmd += p_data.vel_search;
		break;
	case STATE_AUTO_IDLE:
		lcd_state = 'A';
		traj.vel_cmd = 0;
		if(bt1_rise) p_data.state = STATE_AUTO_OPERATE;
		break;
	case STATE_AUTO_OPERATE:
		lcd_state = 'A';
		traj.vel_mode = 1;
		traj.vel_cmd = p_data.vel_feed;
		if(bt1_rise) p_data.state = STATE_AUTO_IDLE;
		if(p_data.is_short) {
			p_data.is_short = 0;
			sg_go_pos(&traj, -1.0, 10);
			p_data.state = STATE_AUTO_RETRACT_UP;
		}
		break;
	case STATE_AUTO_RETRACT_UP:
		lcd_state = 'A';
		if(!traj.active) {
			sg_go_pos(&traj, 0.8, 10);
			p_data.state = STATE_AUTO_RETRACT_DOWN;
		}
		break;
	case STATE_AUTO_RETRACT_DOWN:
		lcd_state = 'A';
		if(!traj.active) p_data.state = STATE_AUTO_OPERATE;
		break;
	case STATE_SHORTCIRC:
		lcd_state = 'E';
		break;
	}
	bt1_rise = 0;
	bt2_rise = 0;

	switch(p_data.var_change_sel){
	case 0: sw_enc_change_val_f(&enc, &p_data.pwm_freq, 100, 1200, 50000);
		break;
	case 1: sw_enc_change_val_f(&enc, &p_data.pwm_dt, 1, 0, 99);
		break;
	case 2: sw_enc_change_val_f(&enc, &p_data.vel_feed, 0.01, 0, 10);
		break;
	case 3: sw_enc_change_val_f(&enc, &p_data.curr_cmd, 0.1, 0, 10);
		break;
	case 4: sw_enc_change_val_f(&enc, &p_data.vel_jog, 1, 0, 99);
		break;
	case 5: sw_enc_change_val_f(&enc, &p_data.vel_search, 1.0, 0, 99);
		break;
	case 6: sw_enc_change_val_f(&enc, &p_data.speed_rot, 1.0, 0, 99);
		break;
	}

	sg_tim2.vel_cmd = p_data.speed_rot;
	//tim calc
	uint32_t arr = (72000000.0) / p_data.pwm_freq;
	if(arr > 65535) {arr = 65535; p_data.pwm_freq = 1200.0f;};
	htim4.Instance->ARR = arr;
	uint32_t ccr = p_data.pwm_dt * 72.0;
	htim4.Instance->CCR1 = ccr;
	//tim calc/

	//display
	memset(disp_buf1, ' ', 16);
	memset(disp_buf2, ' ', 16);
	disp_buf1[16] = 0;
	disp_buf2[16] = 0;
	char edit[] = "*";

	uint32_t time = SysTick->VAL;
	utoa(65535UL, disp_buf1, 10);
	time = time - SysTick->VAL;
	time = time;
	switch(p_data.disp_view){
	case 0:
		if((p_data.var_change_sel >= 0) && (p_data.var_change_sel < 2)); else p_data.var_change_sel = 0;

		if(p_data.var_change_sel == 0) edit[0] = '*'; else edit[0] = 0;
		snprintf(disp_buf1, 12,"%c f=%s%-5.0f  ", lcd_state, edit, p_data.pwm_freq);
		if(p_data.var_change_sel == 1) edit[0] = '*'; else edit[0] = 0;
		snprintf(disp_buf1 + 11, 7,"T=%s%-2.0f", edit, p_data.pwm_dt);
		snprintf(disp_buf2, 9, "If=%-2.1f    ", p_data.curr_fb);
		snprintf(disp_buf2 + 8, 9, "Ul=%-3.0f    ", p_data.volt_load);
		break;
	case 1:
		if((p_data.var_change_sel >= 2) && (p_data.var_change_sel < 4)); else p_data.var_change_sel = 2;

		if(p_data.var_change_sel == 2) edit[0] = '*'; else edit[0] = 0;
		snprintf(disp_buf1, 9,"Vc=%s%-1.2f    ", edit, p_data.vel_feed);
		if(p_data.var_change_sel == 3) edit[0] = '*'; else edit[0] = 0;
		snprintf(disp_buf1 + 8, 8,"Ic=%s%-1.1f    ", edit, p_data.curr_cmd);
		snprintf(disp_buf2, 9, "Vf=%-1.2f    ", p_data.vel_feed_act);
		snprintf(disp_buf2 + 8, 9, "Ub=%-3.0f    ", p_data.volt_bus);
		break;
	case 2:
		if((p_data.var_change_sel >= 4) && (p_data.var_change_sel < 7)); else p_data.var_change_sel = 4;

		if(p_data.var_change_sel == 4) edit[0] = '*'; else edit[0] = 0;
		snprintf(disp_buf1, 9,"Vj=%s%-2f    ", edit, p_data.vel_jog);
		if(p_data.var_change_sel == 5) edit[0] = '*'; else edit[0] = 0;
		snprintf(disp_buf1 + 8, 8,"Vs=%s%-3.0f    ", edit, p_data.vel_search);
		if(p_data.var_change_sel == 6) edit[0] = '*'; else edit[0] = 0;
		snprintf(disp_buf2, 17,"Nr=%s%-3.0f              ", edit, p_data.speed_rot);
		break;
	};

	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(disp_buf1);
	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(disp_buf2);
	//display/

	//adc
	HAL_ADCEx_InjectedStart(&hadc1);
	HAL_ADCEx_InjectedPollForConversion(&hadc1,100);
	adc[0] = HAL_ADCEx_InjectedGetValue(&hadc1,ADC_INJECTED_RANK_1);
	//HAL_ADCEx_InjectedStart(&hadc1);
	HAL_ADCEx_InjectedPollForConversion(&hadc1,100);
	adc[1] = HAL_ADCEx_InjectedGetValue(&hadc1,ADC_INJECTED_RANK_2);
	//HAL_ADCEx_InjectedStart(&hadc1);
	HAL_ADCEx_InjectedPollForConversion(&hadc1,100);
	adc[2] = HAL_ADCEx_InjectedGetValue(&hadc1,ADC_INJECTED_RANK_3);
	HAL_ADCEx_InjectedStop(&hadc1);

	//adc/

};
