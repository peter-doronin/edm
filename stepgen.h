/*
 * stepgen.h
 *
 *  Created on: Oct 27, 2023
 *      Author: cnc
 */

#ifndef INC_STEPGEN_H_
#define INC_STEPGEN_H_

#include "stdbool.h"
#include "stdint.h"
#include "math.h"
#include "stm32f103xb.h"

typedef struct {
	TIM_TypeDef *	tim_base;
	uint32_t *		tim_CCR;
	GPIO_TypeDef * 	Dir_GPIO;
	uint16_t		Dir_GPIO_pin;
	uint32_t tim_CNT;
	uint32_t tim_CNT_prev;
	uint32_t ST_CNT;
	float ST_period;
	uint32_t arr;
	int16_t debug1;
	uint16_t debug2;
	int16_t debug3;
	float 		vel_cmd_abs_cnt;
	float 		pos_act_cnt;
	float 		pos_cmd_prev;
	uint32_t 	pos_cmd_abs_cnt;
	uint32_t 	pos_cmd_abs_cnt_prev;
	uint32_t 	pos_act_abs_cnt;
	uint32_t 	pos_error_cnt;
	float vel_cmd;
	float pos_act;
	float pos_cmd;
	float pos_scale;
	float pos_error;
	float upd_period;
	bool vel_mode;
	bool enable;
	uint16_t step_len;
} sg_tim_t;

typedef struct {
	float pos_cmd;
	float pos_out;
	float pos_fb;
	float pos_error;
	float pos_offset;
	float vel_cmd;
	float vel_tg;
	float vel_out;
	float max_accel;
	float max_vel;
	float upd_period;
	bool enable;
	bool active;
	bool vel_mode;
} sg_traj_t;

void sg_calc_traj(sg_traj_t * inst);
void sg_update_tim(sg_tim_t * inst);
void sg_go_pos(sg_traj_t *inst, float pos, float vel);
int sg_go_pos_block(sg_traj_t *inst, float pos, float vel);

#endif /* INC_STEPGEN_H_ */
