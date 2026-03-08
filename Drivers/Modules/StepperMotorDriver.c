/*
 * StepperMotorDriver.c
 *
 *  Created on: 5 Mar 2026
 *      Author: mehmet_dora
 */


#include <stm32f4xx.h>
#include "stdint.h"
#include "TimerDriver.h"


volatile signed int step_index = 3;
volatile int direction;
volatile uint8_t motor_running;
volatile uint32_t step_delay;



void Stepper_init(void){

	RCC->AHB1ENR |= (1UL << 2);

	// PC0-PC3 mode temizle
	GPIOC->MODER &= ~(0xFF);

	// PC0-PC3 output mode (01)
	GPIOC->MODER |= (1UL << 0) | (1UL << 2) | (1UL << 4) | (1UL << 6);

	direction = 1;

}


void Stepper_start(void){

	motor_running = (uint8_t)1;

	// Tüm pinleri sıfırla
	GPIOC->ODR &= ~(15UL << 0);

	TimerDriver_enable();
}



void Stepper_stop(void){

	motor_running = (uint8_t)0;

	TimerDriver_disable();

	GPIOC->ODR &= ~(15UL << 0);

}
void Stepper_SetDirection(int new_direction){
	direction = new_direction;
}

void Stepper_ToggleDirection(void){
	if(direction == 1){
		direction = 0;
	}else{
		direction = 1;
	}
}
void Stepper_SetSpeed(uint32_t step_delay){

	TimerDriver_disable();
	TIM2->ARR = step_delay - 1;
	TimerDriver_enable();
}




void Stepper_WaveDriveMode(void){


	// Timer interrupt içinde asıl motorun çalışmasını sağlayacak fonk.
	// Bu fonksiyon 2048 kere çalışırsa motor 1 tam tur atacak
	// Her çalışmasında step_index arttırılacak, her 4 çalışmasında step_index sıfırlanacak
	// timer interrupt içinde bu fonksiyonun çağrılma periyoduna göre motorun hızı değişecek


	if(motor_running){

		if(direction){

			step_index--;
			if(step_index == -1){
				step_index = 3;
			}


			//   2		1		0		3		2
			// 0100 -> 0010 -> 0001 -> 1000 -> 0100
			// Önceki durumun bitlerini temizleme
			if(step_index == 0){
				GPIOC->ODR &= ~(1UL << 3);
			}else{
				GPIOC->ODR &= ~(1UL << (step_index - 1));
			}

		}else{


			step_index++;
			if(step_index == 4){
				step_index = 0;
			}

			//   3		2 		1		0		3
			// 1000 <- 0100 <- 0010 <- 0001 <- 1000 pattern şeklinde çalışmalı -> direction == 1 içim
			// Önceki durumun bitlerini temizleme
			if(step_index == 3){
				GPIOC->ODR &= ~(1UL << 0);
			}else{
				GPIOC->ODR &= ~(1UL << (step_index + 1));
			}

		}
	}


	// Yeni durumun bitlerini set etme
	GPIOC->ODR |= (1UL << step_index);

}


void Stepper_FullDriveMode1(void){


	// Timer interrupt içinde asıl motorun çalışmasını sağlayacak fonk.
	// Bu fonksiyon 2048 kere çalışırsa motor 1 tam tur atacak
	// Her çalışmasında step_index arttırılacak, her 4 çalışmasında step_index sıfırlanacak
	// timer interrupt içinde bu fonksiyonun çağrılma periyoduna göre motorun hızı değişecek


	if(motor_running){

		if(direction){

			step_index--;
			if(step_index == -1){
				step_index = 3;
			}


			// Önceki durumun bitlerini temizleme
			if(step_index == 3){
				GPIOC->ODR &= ~(1UL << 0);
			}else{
				GPIOC->ODR &= ~(1UL << (step_index + 1));
			}



			if(step_index == 0){
				// Yeni durumun bitlerini set etme
				GPIOC->ODR |= (1UL << step_index) | (1UL << 3);
			}else{
				// Yeni durumun bitlerini set etme
				GPIOC->ODR |= (1UL << step_index) | (1UL << (step_index - 1));
			}



		}else{


			step_index++;
			if(step_index == 4){
				step_index = 0;
			}


			// Önceki durumun bitlerini temizleme
			if(step_index == 0){
				GPIOC->ODR &= ~(1UL << 3);
			}else{
				GPIOC->ODR &= ~(1UL << (step_index - 1));
			}



			if(step_index == 3){
				// Yeni durumun bitlerini set etme
				GPIOC->ODR |= (1UL << step_index) | (1UL << 0);
			}else{
				// Yeni durumun bitlerini set etme
				GPIOC->ODR |= (1UL << step_index) | (1UL << (step_index + 1));
			}

		}
	}
}


void Stepper_FullDriveMode2(uint8_t index){


	// Timer interrupt içinde asıl motorun çalışmasını sağlayacak fonk.
	// Bu fonksiyon 2048 kere çalışırsa motor 1 tam tur atacak
	// Her çalışmasında step_index arttırılacak, her 4 çalışmasında step_index sıfırlanacak
	// timer interrupt içinde bu fonksiyonun çağrılma periyoduna göre motorun hızı değişecek

	// Clear all coils
	GPIOC->ODR &= ~(15UL << 0);

	if(motor_running){

		if(direction){

			switch(index%4){
				case 0:
					GPIOC->ODR |= (1UL << 3) | (1UL << 2);
					break;

				case 1:
					GPIOC->ODR |= (1UL << 2) | (1UL << 1);
					break;

				case 2:
					GPIOC->ODR |= (1UL << 1) | (1UL << 0);
					break;

				case 3:
					GPIOC->ODR |= (1UL << 0) | (1UL << 3);
					break;

				default:
					GPIOC->ODR &= ~(1UL << 3);
					GPIOC->ODR &= ~(1UL << 2);
					GPIOC->ODR &= ~(1UL << 1);
					GPIOC->ODR &= ~(1UL << 0);
					break;

			}




		}else{


			switch(index%4){
				case 0:
					GPIOC->ODR |= (1UL << 0) | (1UL << 3);
					break;

				case 1:
					GPIOC->ODR |= (1UL << 1) | (1UL << 0);
					break;

				case 2:
					GPIOC->ODR |= (1UL << 2) | (1UL << 1);
					break;

				case 3:
					GPIOC->ODR |= (1UL << 3) | (1UL << 2);
					break;

				default:
					GPIOC->ODR &= ~(1UL << 3);
					GPIOC->ODR &= ~(1UL << 2);
					GPIOC->ODR &= ~(1UL << 1);
					GPIOC->ODR &= ~(1UL << 0);
					break;

			}

		}
	}

}

void Stepper_HalfDriveMode(uint8_t index){


	// Timer interrupt içinde asıl motorun çalışmasını sağlayacak fonk.
	// Bu fonksiyon 2048 kere çalışırsa motor 1 tam tur atacak
	// Her çalışmasında step_index arttırılacak, her 4 çalışmasında step_index sıfırlanacak
	// timer interrupt içinde bu fonksiyonun çağrılma periyoduna göre motorun hızı değişecek

	// Clear all coils
	GPIOC->ODR &= ~(15UL << 0);

	if(motor_running){

		if(direction){

			switch(index%8){
				case 0:
					GPIOC->ODR |= (1UL << 3);
					break;

				case 1:
					GPIOC->ODR |= (1UL << 3) | (1UL << 2);
					break;

				case 2:
					GPIOC->ODR |= (1UL << 2);
					break;

				case 3:
					GPIOC->ODR |= (1UL << 2) | (1UL << 1);
					break;

				case 4:
					GPIOC->ODR |= (1UL << 1) | (1UL << 0);
					break;

				case 5:
					GPIOC->ODR |= (1UL << 0);
					break;

				case 6:
					GPIOC->ODR |= (1UL << 3) | (1UL << 0);
					break;

				case 7:
					GPIOC->ODR |= (1UL << 3);
					break;

				default:
					GPIOC->ODR &= ~(15UL << 0);
					break;
			}




		}else{


			switch(index % 8){

			    case 0:
			        GPIOC->ODR |= (1UL << 3);
			        break;

			    case 1:
			        GPIOC->ODR |= (1UL << 3) | (1UL << 0);
			        break;

			    case 2:
			        GPIOC->ODR |= (1UL << 0);
			        break;

			    case 3:
			        GPIOC->ODR |= (1UL << 1) | (1UL << 0);
			        break;

			    case 4:
			        GPIOC->ODR |= (1UL << 2) | (1UL << 1);
			        break;

			    case 5:
			        GPIOC->ODR |= (1UL << 2);
			        break;

			    case 6:
			        GPIOC->ODR |= (1UL << 3) | (1UL << 2);
			        break;

			    case 7:
			        GPIOC->ODR |= (1UL << 3);
			        break;

			    default:
			        GPIOC->ODR &= ~(15UL << 0);
			        break;
			}

		}
	}

}

