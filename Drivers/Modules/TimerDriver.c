/*
 * TimerDriver.c
 *
 *  Created on: 18 Şub 2026
 *      Author: mehmet_dora
 */



#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "StepperMotorDriver.h"




volatile uint32_t step_counter = 0;



void TimerDriver_enable(void){
	TIM5->CR1 |= (1UL << 0);	// 0. bit olan CEN(Counter Enable) bitini set ederek saymaya başlamasını sağla
}

void TimerDriver_init(void){

	__HAL_RCC_TIM5_CLK_ENABLE(); 	// Timer2 clock enable
	// RCC->AHB1ENR |= (1UL << 0);		// Timer2 clock enable


	// PRESCALER
	TIM5->PSC = (8400 - 1);	// APB1 clock değeri 84 MHz ve 1 ms için -> 8.400 x 10.000
	// 84.000.000 / 8.400 = 10000 hz = 0.1 ms
	// DİKKAT
	// PSC 16 bit tutabilir , yani max PSC değeri 65536 verilebilir !!!


	// Auto-Reloud
	// Kaça kadar saydıktan sonra başa döneceği
	TIM5->ARR = (100 - 1);		// Her 100 saymada bir başa dönsün, her 2.5ms de bir interrupt tetiklenecek


	// OPSİYONEL AYAR
	// TAM burada PSC ve ARR değerleri hemen donanıma yüklenmez , donanım bir update event bekler
	// yani bir cycle boşa gitmiş olur , bunu engellemek için ilk başlamada bu değerlerinin yüklenmesini
	// sağlayan TIM2->EGR(Event Generation) ın UG flag i set edilmelidir
	// Bu şekilde elle event oluşturma ile ilk overflow sırasındaki sayım tam olarak 0'dan başlaması
	// garanti edilmiş olunur.
	TIM5->EGR |= (1UL << 0);
	TIM5->SR &= ~(1UL << 0);	// Olmaması gereken ilk interrupt'ın tetiklenmesini engelleme


	// Interrupt Enable
	TIM5->DIER |= (1UL << 0);	// 0. bit interrupt kontrol eder, 1:enable, 0:disable


	NVIC_EnableIRQ(TIM5_IRQn);	// NVIC ile TIM2 nin interrupt özelliğini aç, CPU'ya erişebilsin
	// Buraya TIM2 enum adı yazılır ki bu sayede TIM2 nin asıl numarası CPU'ya bildirilebilsin



	//TimerDriver_enable();


}



void TimerDriver_disable(void){
	TIM5->CR1 &= ~(1UL << 0);	// 0. bit olan CEN(Counter Enable) bitini clear ederek TIM2 yi durdur
}

void TimerDriver_set_period(uint32_t period){

	TimerDriver_disable();
	TIM5->ARR = period - 1;
	TimerDriver_enable();
}



void TIM5_IRQHandler(void){

	if(TIM5->SR & (1UL << 0)){	// TIM2->SR ın 0. biti UIF flag i dir. Bu set olmuşsa interrupt olmuş demek olur

		// Bu flag counter overflow olduktan ve sıfırlandıktan sonra set edilir.

		// Bu SR içindeki UIF flag'i elle tekrar clear edilmelidir, bu sayede sonraki flag yakalanabilsin.
		// Bu flag yazılım tarafından güncellenmesi gerekir.
		TIM5->SR &= ~TIM_SR_UIF;


		Stepper_HalfDriveMode(step_counter);
		step_counter++;

		if(step_counter == 2048){			// HALF Drive için yarım tur demek
			Stepper_ToggleDirection();
			GPIOA->ODR ^= (1UL << 5);	// LED aç kapa
			step_counter = 0;				// Counter reset
			Stepper_SetSpeed(50);		// TIM2 ARR değiştirerek motor hızının değiştirilmesi
		}


	}


}
