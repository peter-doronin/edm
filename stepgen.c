/*
 * stepgen.c
 *
 *  Created on: Oct 27, 2023
 *      Author: cnc
 */
#include "stepgen.h"
#include "main.h"

void sg_go_pos(sg_traj_t *inst, float pos, float vel){
	inst->vel_mode = 0;
	inst->vel_cmd = vel;
	inst->pos_cmd += pos;
}
int sg_go_pos_block(sg_traj_t *inst, float pos, float vel){
	uint32_t timeout = 10000;
	uint32_t time = uwTick;
	inst->vel_mode = 0;
	inst->vel_cmd = vel;
	inst->pos_cmd += pos;
	while(inst->active){
		if(uwTick > (time + timeout)) return -1;
	}
	return 0;
}

void sg_calc_traj(sg_traj_t *inst) {

	float max_dv = inst->max_accel * inst->upd_period;
	float tiny_dp = max_dv * inst->upd_period;
	float vel_in = fabsf(inst->vel_cmd);
	//pos error and target vel calculation
	if (inst->enable) {
		inst->pos_error = inst->pos_cmd - inst->pos_fb;
		if (inst->pos_error > tiny_dp) {
			inst->vel_tg = -max_dv + sqrtf(2.0 * inst->max_accel * inst->pos_error + max_dv * max_dv);
			inst->active = 1;
		} else if (inst->pos_error < -tiny_dp) {
			inst->vel_tg = max_dv - sqrtf(-2.0 * inst->max_accel * inst->pos_error + max_dv * max_dv);
			inst->active = 1;
		} else {
			inst->vel_tg = 0.0;
			inst->active = 0;
		}
	} else {
		inst->vel_tg = 0.0;
	}
	//pos error and target vel calculation/


	//velocity limitation by vel_in
	if (inst->vel_tg > vel_in) {
		inst->vel_tg = vel_in;
	} else if (inst->vel_tg < -vel_in) {
		inst->vel_tg = -vel_in;
	}
	//velocity limitation by vel_in/

	//velocity mode switch
	if(inst->vel_mode) {
		inst->vel_tg = inst->vel_cmd;
		inst->pos_cmd = inst->pos_fb;
	}
	//velocity mode switch

	/* ramp velocity toward request at accel limit */
	if (inst->vel_tg > inst->vel_out + max_dv) {
		inst->vel_out += max_dv;
	} else if (inst->vel_tg < inst->vel_out - max_dv) {
		inst->vel_out -= max_dv;
	} else {
		inst->vel_out = inst->vel_tg;
	}

	if (inst->vel_out != 0.0) /* check for still moving */
	{
		inst->active = 1;
	}
	inst->pos_out += inst->vel_out; /* integrate velocity to get new position */

}
;

void sg_update_tim(sg_tim_t *inst) {
	inst->tim_CNT = inst->tim_base->CNT;

	if(inst->vel_mode) inst->vel_cmd_abs_cnt = fabsf(inst->vel_cmd * inst->pos_scale * inst->upd_period);
	else {
		inst->vel_cmd = inst->pos_cmd - inst->pos_cmd_prev;
		inst->vel_cmd_abs_cnt = fabsf(inst->vel_cmd * inst->pos_scale);
	}

	inst->pos_cmd_abs_cnt += (uint32_t)(inst->vel_cmd_abs_cnt * 0xffff);

	if (inst->vel_cmd_abs_cnt != 0) {
		bool dir;
		if(inst->vel_cmd > 0) dir = 1; else dir = 0;

		if (inst->vel_cmd_abs_cnt > 1) {

			uint16_t calc_counts = ((65535 + inst->arr - inst->tim_CNT_prev) / inst->arr);
			inst->pos_act_abs_cnt += calc_counts;
			if (dir) {
				HAL_GPIO_WritePin(inst->Dir_GPIO, inst->Dir_GPIO_pin, GPIO_PIN_SET);
				inst->pos_act_cnt += calc_counts;
			} else {
				HAL_GPIO_WritePin(inst->Dir_GPIO, inst->Dir_GPIO_pin, GPIO_PIN_RESET);
				inst->pos_act_cnt -= calc_counts;
			}

			inst->arr = 0xffff / (inst->vel_cmd_abs_cnt);
			if (inst->arr < 200) inst->arr = 200;
			inst->tim_base->ARR = inst->arr;
			*inst->tim_CCR = inst->arr - 100;
			inst->tim_base->CR1 &= ~TIM_CR1_OPM;
			inst->tim_base->CR1 |= TIM_CR1_CEN;

		} else {
			if ((inst->pos_cmd_abs_cnt & 0xffff0000) != (inst->pos_cmd_abs_cnt_prev & 0xffff0000)) {
				inst->arr = (inst->pos_cmd_abs_cnt_prev & 0xffff);
				inst->arr = (~inst->arr) & 0xffff;
				inst->arr = inst->arr / inst->vel_cmd_abs_cnt;
				if (inst->arr > 0xffff) inst->arr = 0xffff;
				uint16_t systick = ~((uint16_t) SysTick->VAL);
				if (inst->arr < 102) inst->arr = 100;
				if ((inst->arr + 100) < systick) inst->arr = systick + 102;
				*inst->tim_CCR &= ~TIM_CR1_CEN;
				inst->tim_base->CNT = systick - 300;
				inst->tim_base->ARR = inst->arr;
				*inst->tim_CCR = inst->arr - 100;
				inst->tim_base->CR1 |= TIM_CR1_OPM;
				inst->tim_base->CR1 |= TIM_CR1_CEN;

				inst->pos_act_abs_cnt += 1;

				if (dir) {
					HAL_GPIO_WritePin(inst->Dir_GPIO, inst->Dir_GPIO_pin, GPIO_PIN_SET);
					inst->pos_act_cnt += 1;
				} else {
					HAL_GPIO_WritePin(inst->Dir_GPIO, inst->Dir_GPIO_pin, GPIO_PIN_RESET);
					inst->pos_act_cnt -= 1;
				}

			} else {
				inst->tim_base->CR1 &= ~TIM_CR1_OPM;
				inst->tim_base->CR1 &= ~TIM_CR1_CEN;
				*inst->tim_CCR = 0;
				inst->tim_base->ARR = 65535;
			}
		}

	} else { // zero speed
		inst->tim_base->CR1 &= ~TIM_CR1_OPM;
		inst->tim_base->CR1 &= ~TIM_CR1_CEN;
		inst->tim_base->CNT = 0;
		inst->tim_base->ARR = 65535;
	}

	inst->pos_act = inst->pos_act_cnt / inst->pos_scale;
	inst->pos_cmd_prev = inst->pos_cmd;
	inst->pos_error = inst->pos_cmd - inst->pos_act;
	inst->pos_cmd_abs_cnt_prev = inst->pos_cmd_abs_cnt;
	inst->tim_CNT_prev = inst->tim_CNT;
}

