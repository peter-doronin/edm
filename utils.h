/*
 * utils.h
 *
 *  Created on: 14 нояб. 2023 г.
 *      Author: user
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "stdbool.h"
#include "stdint.h"
#include "stm32f1xx_hal.h"

typedef struct {
	GPIO_TypeDef * port_A;
	uint16_t 	pin_A;
	GPIO_TypeDef * port_B;
	uint16_t 	pin_B;
	bool 		phase_A;
	bool 		phase_B;
	uint8_t 	state_AB;
	uint8_t 	state_AB_prev;
	uint32_t	pos;
	int16_t 	pos_inc;
	int16_t 	val_inc;
	bool 		change_val_int_ena;
	bool 		change_val_uint_ena;
	bool 		change_val_f_ena;
	uint16_t * 	val_uint;
	uint16_t 	val_uint_scale;
	int16_t * 	val_int;
	int16_t 	val_int_scale;
	float * 	val_f;
	float 		val_f_scale;
	float 		val_min_f;
	float 		val_max_f;

} sw_encoder_t;


void sw_enc_loop(sw_encoder_t * inst);
void sw_enc_change_val_uint(sw_encoder_t * inst, uint16_t * val, uint16_t scale);
void sw_enc_change_val_int(sw_encoder_t * inst, int16_t * val, int16_t scale);
void sw_enc_change_val_f(sw_encoder_t * inst, float * val, float scale, float min, float max);
void sw_enc_change_val_stop(sw_encoder_t * inst);

#endif /* INC_UTILS_H_ */

