/*
 * process.h
 *
 *  Created on: 18 нояб. 2023 г.
 *      Author: user
 */

#ifndef INC_PROCESS_H_
#define INC_PROCESS_H_

#include <stdbool.h>
#include <stdint.h>

enum {
	STATE_JOG,
	STATE_SEARCH,
	STATE_AUTO_IDLE,
	STATE_AUTO_OPERATE,
	STATE_AUTO_RETRACT_UP,
	STATE_AUTO_RETRACT_DOWN,
	STATE_SHORTCIRC
} STATE;

typedef struct {
	bool is_enable;
	bool is_auto;
	bool is_curr_zero;
	bool is_curr_over;
	bool is_volt_noload;
	bool is_volt_short;
	bool is_short;
	float pwm_freq;			//set 0
	float pwm_dt;			//set 1
	float volt_load;		//disp
	float curr_fb;			//disp

	float vel_feed;			//set 2
	float curr_cmd;			//set 3
	float vel_feed_act;		//disp
	float volt_bus;  		//disp
	float vel_jog;			//set 4
	float vel_search;		//set 5
	float speed_rot;		//set 6
	uint16_t var_change_sel;
	uint16_t pwm_arr;
	uint16_t pwm_ccr;
	uint16_t state;
	uint16_t disp_view;
} process_data_t;

void process_loop(void);

#endif /* INC_PROCESS_H_ */
