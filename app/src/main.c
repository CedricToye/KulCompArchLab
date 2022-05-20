#include <stdint.h>
#include <stm32l4xx.h>
// Cédric Toye

float temperatuur;
float value;
float V;
float R;

void delay(unsigned int n) {
	volatile unsigned int delay = n;
	while (delay--)
		;
}

void SysTick_Handler(void) {
	switch (mux) {
	case 0:
		clear();
		GPIOA->ODR &= ~GPIO_ODR_OD8;
		GPIOA->ODR &= ~GPIO_ODR_OD15;
		segments( / 1000);
		break;
	case 1:
		clear();
		GPIOA->ODR |= GPIO_ODR_OD8;
		GPIOA->ODR &= ~GPIO_ODR_OD15;
		segments(( / 100) % 10);
		break;
	case 2:
		clear();
		GPIOA->ODR |= GPIO_ODR_OD6;
		GPIOA->ODR &= ~GPIO_ODR_OD8;
		GPIOA->ODR |= GPIO_ODR_OD15;
		segments(( % 100) / 10);
		break;
	case 3:
		clear();
		GPIOA->ODR |= GPIO_ODR_OD8;
		GPIOA->ODR |= GPIO_ODR_OD15;
		segments(( % 100) % 10);
		break;
	}

	mux++;

	if (mux > 3) {
		mux = 0;
	}

}

void clear(void) {
	GPIOA->ODR &= ~(GPIO_ODR_OD7 | GPIO_ODR_OD5 | GPIO_ODR_OD6);
	GPIOB->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD12 | GPIO_ODR_OD15 | GPIO_ODR_OD1
			| GPIO_ODR_OD2);
}


int main(void) {

	SysTick_Config(48000);

	NVIC_SetPriority(SysTick_IRQn, 128);
	NVIC_EnableIRQ(SysTick_IRQn);

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk; // Activating clock block A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk; // Activating clock block B
	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN; // Activating clock I2C

	// Klok selecteren voor ADC
	RCC->CCIPR &= ~RCC_CCIPR_ADCSEL_Msk;
	RCC->CCIPR |= (RCC_CCIPR_ADCSEL_0 | RCC_CCIPR_ADCSEL_1);

	//Setting GPIO
	GPIOB->MODER &= ~GPIO_MODER_MODE6_Msk;
	GPIOB->MODER |=  GPIO_MODER_MODE6_1;
	GPIOB->OTYPER |= GPIO_OTYPER_OT6;
	GPIOB->AFR[0] = (GPIOB->AFR[0] & (~GPIO_AFRL_AFSEL6_Msk)) | (0x4 << GPIO_AFRL_AFSEL6_Pos);

	GPIOB->MODER &= ~GPIO_MODER_MODE7_Msk;
	GPIOB->MODER |=  GPIO_MODER_MODE7_1;
	GPIOB->OTYPER |= GPIO_OTYPER_OT7;
	GPIOB->AFR[0] = (GPIOB->AFR[0] & (~GPIO_AFRL_AFSEL7_Msk)) | (0x4 << GPIO_AFRL_AFSEL7_Pos);

	//I2C module instellen, voor 100kHz clock
	I2C1->TIMINGR = 0x20303E5D;
	I2C1->CR2 |= (I2C_CR2_AUTOEND | I2C_CR2_NACK);
	I2C1->CR1 |= I2C_CR1_PE;

	//setting segments
	GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk
			| GPIO_MODER_MODE7_Msk); // 7-B, 5-C, 6-DP
	GPIOA->MODER |= (GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0
			| GPIO_MODER_MODE7_0);
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT5 | GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);

	GPIOB->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk
			| GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE12_Msk
			| GPIO_MODER_MODE15_Msk); // 0-A, 12-D, 15-E, 1-F, 2-G
	GPIOB->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0
			| GPIO_MODER_MODE2_0 | GPIO_MODER_MODE12_0 | GPIO_MODER_MODE15_0);
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2
			| GPIO_OTYPER_OT12 | GPIO_OTYPER_OT15);

	while (1) {


	}

}
