/*
 * utils.c
 *
 *  Created on: 14 нояб. 2023 г.
 *      Author: user
 */

#include "utils.h"
#include "stdint.h"


void sw_enc_loop(sw_encoder_t * inst){
	inst->phase_A = (inst->port_A->IDR & inst->pin_A);
	inst->phase_B = (inst->port_B->IDR & inst->pin_B);
	inst->state_AB = inst->phase_A | (inst->phase_B << 1);
	inst->pos_inc = 0;
	switch(inst->state_AB_prev){
	case 0: if(inst->state_AB == 1) inst->pos_inc = 1;
			if(inst->state_AB == 2) inst->pos_inc = -1;
			break;
	case 1: if(inst->state_AB == 3) inst->pos_inc = 1;
			if(inst->state_AB == 0) inst->pos_inc = -1;
			break;
	case 3: if(inst->state_AB == 2) inst->pos_inc = 1;
			if(inst->state_AB == 1) inst->pos_inc = -1;
			break;
	case 2: if(inst->state_AB == 0) inst->pos_inc = 1;
			if(inst->state_AB == 3) inst->pos_inc = -1;
			break;
	}
	inst->state_AB_prev = inst->state_AB;

	inst->pos += inst->pos_inc;
	inst->val_inc += inst->pos_inc;

	if((inst->val_inc > 1) || ((inst->val_inc < -1)))  {
		if(inst->val_inc > 0) {
			if(inst->change_val_f_ena) {
				*inst->val_f += inst->val_f_scale;
				if(*inst->val_f > inst->val_max_f) *inst->val_f = inst->val_max_f;
			};
			if(inst->change_val_int_ena) {*inst->val_int += inst->val_int_scale;};
			if(inst->change_val_uint_ena) {
				if(((uint32_t)*inst->val_uint + (uint32_t)inst->val_uint_scale) <= 0xffff) *inst->val_uint += inst->val_uint_scale;
			};
			} else {
			if(inst->change_val_f_ena) {
				*inst->val_f -= inst->val_f_scale;
				if(*inst->val_f < inst->val_min_f) *inst->val_f = inst->val_min_f;
			};
			if(inst->change_val_int_ena) {*inst->val_uint -= inst->val_int_scale;};
			if(inst->change_val_uint_ena) {
				if((uint32_t)*inst->val_uint >= (uint32_t)inst->val_uint_scale) *inst->val_uint -= inst->val_uint_scale;
			}
			};
		inst->val_inc = 0;
		};
}

void sw_enc_change_val_uint(sw_encoder_t * inst, uint16_t * val, uint16_t scale){
	inst->change_val_uint_ena = 1;
	inst->val_uint = val;
	inst->val_uint_scale = scale;
}

void sw_enc_change_val_int(sw_encoder_t * inst, int16_t * val, int16_t scale){
	inst->change_val_int_ena = 1;
	inst->val_int = val;
	inst->val_int_scale = scale;
}

void sw_enc_change_val_f(sw_encoder_t * inst, float * val, float scale, float min, float max){
	inst->change_val_f_ena = 1;
	inst->val_f = val;
	inst->val_f_scale = scale;
	inst->val_min_f = min;
	inst->val_max_f = max;
}

void sw_enc_change_val_stop(sw_encoder_t * inst){
	inst->change_val_f_ena = 0;
	inst->change_val_uint_ena = 0;
	inst->change_val_int_ena = 0;
}
