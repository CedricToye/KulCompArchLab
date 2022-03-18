#include <stdint.h>
#include <stm32l4xx.h>
// Cédric Toye

int mux = 0;
int uren = 0;
int minuten = 0;
int ms = 0;

//void EXTI15_10_Handler(void){
//	if (EXTI->PR1 & EXTI_PR1_PIF13) {
//		uren++;
//		}

//	else if (EXTI->PR1 & EXTI_PR1_PIF14){
//		minuten++;
//		}
//	}

void delay(unsigned int n){
    volatile unsigned int delay = n;
    while (delay--);
}

void SysTick_Handler(void) {
	switch(mux){
		case 0:
			clear();
			GPIOA->ODR &= ~GPIO_ODR_OD8;
			GPIOA->ODR &= ~GPIO_ODR_OD15;
			segments(uren/10);
			break;
		case 1:
			clear();
			GPIOA->ODR |= GPIO_ODR_OD6;
			GPIOA->ODR |= GPIO_ODR_OD8;
			GPIOA->ODR &= ~GPIO_ODR_OD15;
			segments(uren%10);
			break;
		case 2:
			clear();
			GPIOA->ODR &= ~GPIO_ODR_OD8;
			GPIOA->ODR |= GPIO_ODR_OD15;
			segments(minuten/10);
			break;
		case 3:
			clear();
			GPIOA->ODR |= GPIO_ODR_OD8;
			GPIOA->ODR |= GPIO_ODR_OD15;
			segments(minuten%10);
			break;
	}

	mux++;
	ms++;

	if(mux > 3){
		mux = 0;
	}

	if(ms == 60000){
		ms = 0;
		minuten++;
		if(minuten > 59){
			minuten = 0;
			uren++;
			if(uren > 23){
				uren = 0;
			}
		}
	}

}

void clear(void){
	GPIOA->ODR &= ~(GPIO_ODR_OD7|GPIO_ODR_OD5|GPIO_ODR_OD6);
	GPIOB->ODR &= ~(GPIO_ODR_OD0|GPIO_ODR_OD12|GPIO_ODR_OD15|GPIO_ODR_OD1|GPIO_ODR_OD2);
}

void segments(unsigned int n){
	switch(n){
		case 0:
			GPIOA->ODR |= (GPIO_ODR_OD5|GPIO_ODR_OD7);
			GPIOB->ODR |= (GPIO_ODR_OD0|GPIO_ODR_OD1|GPIO_ODR_OD12|GPIO_ODR_OD15);
			break;

		case 1:
			GPIOA->ODR |= (GPIO_ODR_OD5|GPIO_ODR_OD7);
			break;

		case 2:
			GPIOA->ODR |= (GPIO_ODR_OD7);
			GPIOB->ODR |= (GPIO_ODR_OD0|GPIO_ODR_OD2|GPIO_ODR_OD12|GPIO_ODR_OD15);
			break;

		case 3:
			GPIOA->ODR |= (GPIO_ODR_OD5|GPIO_ODR_OD7);
			GPIOB->ODR |= (GPIO_ODR_OD0|GPIO_ODR_OD2|GPIO_ODR_OD12);
			break;

		case 4:
			GPIOA->ODR |= (GPIO_ODR_OD5|GPIO_ODR_OD7);
			GPIOB->ODR |= (GPIO_ODR_OD1|GPIO_ODR_OD2);
			break;

		case 5:
			GPIOA->ODR |= (GPIO_ODR_OD5);
			GPIOB->ODR |= (GPIO_ODR_OD0|GPIO_ODR_OD1|GPIO_ODR_OD2|GPIO_ODR_OD12);
			break;

		case 6:
			GPIOA->ODR |= GPIO_ODR_OD5;
			GPIOB->ODR |= (GPIO_ODR_OD0|GPIO_ODR_OD1|GPIO_ODR_OD2|GPIO_ODR_OD12|GPIO_ODR_OD15);
			break;

		case 7:
			GPIOA->ODR |= (GPIO_ODR_OD5|GPIO_ODR_OD7);
			GPIOB->ODR |= GPIO_ODR_OD0;
			break;
		case 8:
			GPIOA->ODR |= (GPIO_ODR_OD5|GPIO_ODR_OD7);
			GPIOB->ODR |= (GPIO_ODR_OD0|GPIO_ODR_OD1|GPIO_ODR_OD2|GPIO_ODR_OD12|GPIO_ODR_OD15);
			break;

		case 9:
			GPIOA->ODR |= (GPIO_ODR_OD5|GPIO_ODR_OD7);
			GPIOB->ODR |= (GPIO_ODR_OD0|GPIO_ODR_OD1|GPIO_ODR_OD2|GPIO_ODR_OD12);
			break;
	}
}

int main(void) {

	SysTick_Config(48000);

	NVIC_SetPriority(SysTick_IRQn, 128);
	NVIC_EnableIRQ(SysTick_IRQn);

//	NVIC_SetPriority(EXTI15_10_IRQn, 129);
//	NVIC_EnableIRQ(EXTI15_10_IRQn);

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk; // Activating clock block A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk; // Activating clock block B
//	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	//setting multiplexer
	GPIOA->MODER &= ~(GPIO_MODER_MODE8_Msk|GPIO_MODER_MODE15_Msk); // bitwise and operation of GPIOA MODER register and mask
	GPIOA->MODER |= (GPIO_MODER_MODE8_0|GPIO_MODER_MODE15_0); // bitwise or operation of GPIOA MODER register and 01 bits for pin 8 and pin 15
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT8|GPIO_OTYPER_OT15);

	//setting segments
	GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk|GPIO_MODER_MODE6_Msk|GPIO_MODER_MODE7_Msk); // 7-B, 5-C, 6-DP
	GPIOA->MODER |= (GPIO_MODER_MODE5_0|GPIO_MODER_MODE6_0|GPIO_MODER_MODE7_0);
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT5|GPIO_OTYPER_OT6|GPIO_OTYPER_OT7);

	GPIOB->MODER &= ~(GPIO_MODER_MODE0_Msk|GPIO_MODER_MODE1_Msk|GPIO_MODER_MODE2_Msk|GPIO_MODER_MODE12_Msk|GPIO_MODER_MODE15_Msk); // 0-A, 12-D, 15-E, 1-F, 2-G
	GPIOB->MODER |= (GPIO_MODER_MODE0_0|GPIO_MODER_MODE1_0|GPIO_MODER_MODE2_0|GPIO_MODER_MODE12_0|GPIO_MODER_MODE15_0);
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT0|GPIO_OTYPER_OT1|GPIO_OTYPER_OT2|GPIO_OTYPER_OT12|GPIO_OTYPER_OT15);

	//setting buttons
	GPIOB->MODER &= ~GPIO_MODER_MODE13_Msk; // Button A
	GPIOB->MODER &= ~GPIO_MODER_MODE14_Msk; // Button B

	//setting pull up resistors
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD13_Msk;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD13_0;
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD14_Msk;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD14_0;

	//Buttons routeren naar de EXTI
//	SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13_Msk;
//	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PB;
//	SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI14_Msk;
//	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI14_PB;

	//Falling edge interrupt aanzetten
//	EXTI->FTSR1 |= EXTI_FTSR1_FT13;
//	EXTI->IMR1 |= EXTI_IMR1_IM13;
//	EXTI->FTSR1 |= EXTI_FTSR1_FT14;
//	EXTI->IMR1 |= EXTI_IMR1_IM14;

	while (1) {
        if (!(GPIOB->IDR  & GPIO_IDR_ID13)) {
            minuten++;
            if(minuten > 59){
				minuten = 0;
            }
            delay(1000000);
        }

        if (!(GPIOB->IDR & GPIO_IDR_ID14)){
            uren++;
            if(uren > 23){
				uren = 0;
			}
            delay(1000000);
            }
	}

}
