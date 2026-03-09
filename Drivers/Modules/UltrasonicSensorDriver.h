/*
 * UltrasonicSensorDriver.h
 *
 *  Created on: 7 Mar 2026
 *      Author: mehmet_dora
 */

#ifndef MODULES_ULTRASONICSENSORDRIVER_H_
#define MODULES_ULTRASONICSENSORDRIVER_H_

	#include <stdint.h>
	void HCSR_04_init(void);
	void HCSR_04_start(void);
	void HCSR_04_stop(void);
	uint32_t HCSR_04_GetDistance(void);
	void delay_us(uint32_t us);
	extern volatile uint8_t measurement_done;

#endif /* MODULES_ULTRASONICSENSORDRIVER_H_ */
