#include <stdint.h>
#include <stm32l4xx.h>
// Cédric Toye

int mux = 0;
int temperatuur = 0;

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
		segments(temperatuur / 1000);
		break;
	case 1:
		clear();
		GPIOA->ODR |= GPIO_ODR_OD8;
		GPIOA->ODR &= ~GPIO_ODR_OD15;
		segments((temperatuur / 100) % 10);
		break;
	case 2:
		clear();
		GPIOA->ODR &= ~GPIO_ODR_OD8;
		GPIOA->ODR |= GPIO_ODR_OD15;
		segments((temperatuur % 100) / 10);
		break;
	case 3:
		clear();
		GPIOA->ODR |= GPIO_ODR_OD8;
		GPIOA->ODR |= GPIO_ODR_OD15;
		segments((temperatuur % 100) % 10);
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

void segments(unsigned int n) {
	switch (n) {
	case 0:
		GPIOA->ODR |= (GPIO_ODR_OD5 | GPIO_ODR_OD7);
		GPIOB->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD12
				| GPIO_ODR_OD15);
		break;

	case 1:
		GPIOA->ODR |= (GPIO_ODR_OD5 | GPIO_ODR_OD7);
		break;

	case 2:
		GPIOA->ODR |= (GPIO_ODR_OD7);
		GPIOB->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD2 | GPIO_ODR_OD12
				| GPIO_ODR_OD15);
		break;

	case 3:
		GPIOA->ODR |= (GPIO_ODR_OD5 | GPIO_ODR_OD7);
		GPIOB->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD2 | GPIO_ODR_OD12);
		break;

	case 4:
		GPIOA->ODR |= (GPIO_ODR_OD5 | GPIO_ODR_OD7);
		GPIOB->ODR |= (GPIO_ODR_OD1 | GPIO_ODR_OD2);
		break;

	case 5:
		GPIOA->ODR |= (GPIO_ODR_OD5);
		GPIOB->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2
				| GPIO_ODR_OD12);
		break;

	case 6:
		GPIOA->ODR |= GPIO_ODR_OD5;
		GPIOB->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2
				| GPIO_ODR_OD12 | GPIO_ODR_OD15);
		break;

	case 7:
		GPIOA->ODR |= (GPIO_ODR_OD5 | GPIO_ODR_OD7);
		GPIOB->ODR |= GPIO_ODR_OD0;
		break;
	case 8:
		GPIOA->ODR |= (GPIO_ODR_OD5 | GPIO_ODR_OD7);
		GPIOB->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2
				| GPIO_ODR_OD12 | GPIO_ODR_OD15);
		break;

	case 9:
		GPIOA->ODR |= (GPIO_ODR_OD5 | GPIO_ODR_OD7);
		GPIOB->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2
				| GPIO_ODR_OD12);
		break;
	}
}

int main(void) {

	SysTick_Config(48000);

	NVIC_SetPriority(SysTick_IRQn, 128);
	NVIC_EnableIRQ(SysTick_IRQn);

	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN; // Activating clock ADC
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk; // Activating clock block A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk; // Activating clock block B

	// Klok selecteren
	RCC->CCIPR &= ~RCC_CCIPR_ADCSEL_Msk;
	RCC->CCIPR |= (RCC_CCIPR_ADCSEL_0 | RCC_CCIPR_ADCSEL_1);

	// Deep powerdown modus uitzetten
	ADC1->CR &= ~ADC_CR_DEEPPWD;

	// ADC voltage regulator aanzetten
	ADC1->CR |= ADC_CR_ADVREGEN;

	// Delay 20 microseconds
	delay(1500);

	// Kalibratie starten
	ADC1->CR |= ADC_CR_ADCAL;
	while(ADC1->CR & ADC_CR_ADCAL);

	//ADC aanzetten
	ADC1->CR |= ADC_CR_ADEN;

	// Kanalen instellen
	ADC1->SMPR1 |= (ADC_SMPR1_SMP0_0 | ADC_SMPR1_SMP0_1 | ADC_SMPR1_SMP0_2);
	ADC1->SQR1 |= (ADC_SQR1_SQ1_0 | ADC_SQR1_SQ1_2);

	//Setting NTC as analog
	GPIOA->MODER &= ~GPIO_MODER_MODE0_Msk; // bits op 0 zetten
	GPIOA->MODER |= GPIO_MODER_MODE0_0 | GPIO_MODER_MODE0_1; // Bit 0 en 1 hoog zetten voor analoge modus

	//setting multiplexer
	GPIOA->MODER &= ~(GPIO_MODER_MODE8_Msk | GPIO_MODER_MODE15_Msk); // bitwise and operation of GPIOA MODER register and mask
	GPIOA->MODER |= (GPIO_MODER_MODE8_0 | GPIO_MODER_MODE15_0); // bitwise or operation of GPIOA MODER register and 01 bits for pin 8 and pin 15
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT8 | GPIO_OTYPER_OT15);

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
		 // Start de ADC en wacht tot de sequentie klaar is
		 ADC1->CR |= ADC_CR_ADSTART;
		 while(!(ADC1->ISR & ADC_ISR_EOS));

		 // Lees de waarde in
		 value = ADC1->DR;

		 delay(1000000);
	}

}
