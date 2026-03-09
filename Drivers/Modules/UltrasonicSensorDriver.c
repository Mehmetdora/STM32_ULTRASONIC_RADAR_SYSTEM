/*
 * UltrasonicSensorDriver.c
 *
 *  Created on: 7 Mar 2026
 *      Author: mehmet_dora
 */


#include <stdint.h>
#include <stm32f4xx.h>

volatile uint32_t start_time;
volatile uint32_t stop_time;
volatile uint32_t pulse_width;
volatile uint8_t edge = 0;
volatile uint8_t measurement_done = 0;


void delay_us(uint32_t us);

void HCSR_04_init(void){

	// Trig ve Echo pinlerinin ayarlanması

	RCC->AHB1ENR |= (1UL << 0);		// GPIOA clock enable

	GPIOA->MODER &= ~(3UL << 0);
	GPIOA->MODER |= (1UL << 0);		// PA0 output mode

	GPIOA->PUPDR &= ~(3UL << 0);	// PA0 no pupd

	GPIOA->MODER &= ~(3UL << 2);
	GPIOA->MODER |= (2UL << 2);		// PA1 anlternate func. mode

	GPIOA->AFR[0] &= ~(15UL << 4);
	GPIOA->AFR[0] |= (1UL << 4);		// PA1 için AF1 ayarlanması , yani TIM2 kullanılacak



	// TIM3 ayarları
	// Trig için gerekli olan 10 mikrosaniye için kullanılacak TIM3 ayarlaması
	RCC->APB1ENR |= (1UL << 1);

	TIM3->PSC = 84 - 1;
	TIM3->ARR = 0xFFFFFFFF;			// 1 mikrosaniye period ile sayacak

	TIM3->CR1 |= (1UL << 0);		// start counter




	// TIM2  ayarları
	RCC->APB1ENR |= (1UL << 0);		// TIM2 clock enable

	TIM2->PSC = 84 - 1;				// 84 Mhz / 84 = 1MHz
	TIM2->ARR = 0xFFFFFFFF;			// 1 clock = 1 mikro saniye , echo ile gelecek sinyalin genişliğini ölçmek için

	TIM2->CCMR1 &= ~(3UL << 8);
	TIM2->CCMR1 |= (1UL << 8);		// Chanel 2 nin input capture yapılması

	TIM2->CCER &= ~(1UL << 5);		// rising edge, echo pininden gelen rising edge yakalamak için

	TIM2->CCER |= (1UL << 4);		// enable capture

	TIM2->DIER |= (1UL << 2);		// interrupt enable, herhangi bir edge yakalandığında tetiklenecek

	NVIC_EnableIRQ(TIM2_IRQn);

	TIM2->CR1 |= (1UL << 0);		// counter enable, timer saymaya başlar




}



void HCSR_04_start(void){

	GPIOA->BSRR = (1UL << 0);   // TRIG pinini HIGH yap
	delay_us(10);				// 10 mikrosaniye delay
	GPIOA->BSRR = (1UL << 16);  // TRIG pinini LOW yap
}

void HCSR_04_stop(void){
	TIM2->CR1 &= ~(1UL << 0);
}

uint32_t HCSR_04_GetDistance(void){
	return pulse_width / 58;
}




// Bu delay fonk.  sensörün ölçüme başlaması için  high durumunda beklenmesi gereken 10 mikrosaniyelik delay için kullanılacak
// Bu delay için farklı bir timer kullanıldı çünkü CNT değerinin delay ve input capture için aynı anda kullanımı hatalı ölçüme neden oluyor.
void delay_us(uint32_t us)
{
    uint32_t start = TIM3->CNT;

    while((TIM3->CNT - start) < us);
}


void TIM2_IRQHandler(void){

	if(TIM2->SR & (1UL << 2))   // CC2 interrupt status
	{

		/*
		 * Rising edge gelirse interupt tetiklenecek ve henüz edge 0 olduğundan bu zaman
		 * kaydedilecek, sonrasıda fallaing edge beklemek için TIM2 buna göre ayarlanıyor.
		 *
		 * Falling edge gelirse interrupt tetiklenecek ve edge 1 olduğundan stop time kaydedilecek,
		 * start ve stop arasında geçen zaman farkı alınıp bir sonraki rising edge beklemek için
		 * TIM2 ayarlanacak.
		 *
		 * Her interrupt sonunda da ayırca SR flag clear ediliyor.
		 */
		if(edge == 0){

			start_time = TIM2->CCR2;
			edge = 1;

			TIM2->CCER |= (1UL << 5); // falling edge

		}else{

			stop_time = TIM2->CCR2;


			if(stop_time >= start_time)
				pulse_width = stop_time - start_time;
			else
				pulse_width = (0xFFFFFFFF - start_time) + stop_time;

			measurement_done = 1;

			edge = 0;

			TIM2->CCER &= ~(1UL << 5); // rising edge
		}

	TIM2->SR &= ~(1UL << 2);	// Reset CC2 interrupt status


	}


}
