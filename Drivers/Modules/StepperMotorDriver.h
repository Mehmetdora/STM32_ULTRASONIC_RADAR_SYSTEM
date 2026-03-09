/*
 * StepperMotorDriver.h
 *
 *  Created on: 5 Mar 2026
 *      Author: mehmet_dora
 */

#ifndef MODULES_STEPPERMOTORDRIVER_H_
#define MODULES_STEPPERMOTORDRIVER_H_

	#include "stdint.h"

	extern volatile int direction;

	void Stepper_init(void);
	void Stepper_start(void);
	void Stepper_stop(void);
	void Stepper_SetDirection(int new_direction);
	void Stepper_ToggleDirection(void);
	void Stepper_SetSpeed(uint32_t step_delay);
	uint16_t Stepper_GetAngle(void);
	void Stepper_WaveDriveMode(void);
	void Stepper_FullDriveMode1(void);
	void Stepper_FullDriveMode2(uint8_t index);
	void Stepper_HalfDriveMode(uint8_t index);


#endif /* MODULES_STEPPERMOTORDRIVER_H_ */
