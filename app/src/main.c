#include <stdint.h>
#include <stm32l4xx.h>
// Cédric Toye

int tick = 0;
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
	tick++;
}

int __io_putchar(int ch){
    while(!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = ch;
}

int main(void) {

	SysTick_Config(48000);

	NVIC_SetPriority(SysTick_IRQn, 128);
	NVIC_EnableIRQ(SysTick_IRQn);

	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN; // Activating clock ADC
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk; // Activating clock block A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk; // Activating clock block B
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN; //Activating clock block UART

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
	ADC1->SMPR1 |= (ADC_SMPR1_SMP5_0 | ADC_SMPR1_SMP5_1 | ADC_SMPR1_SMP5_2);
	ADC1->SQR1 |= (ADC_SQR1_SQ1_0 | ADC_SQR1_SQ1_2);

	//Setting NTC as analog
	GPIOA->MODER &= ~GPIO_MODER_MODE0_Msk; // bits op 0 zetten
	GPIOA->MODER |= GPIO_MODER_MODE0_0 | GPIO_MODER_MODE0_1; // Bit 0 en 1 hoog zetten voor analoge modus

	//setting GPIO
	GPIOA->MODER &= ~GPIO_MODER_MODE9_Msk;
	GPIOA->MODER |=  GPIO_MODER_MODE9_1;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT9;
	GPIOA->AFR[1] = (GPIOA->AFR[1] & (~GPIO_AFRH_AFSEL9_Msk)) | (0x7 << GPIO_AFRH_AFSEL9_Pos);

	GPIOA->MODER &= ~GPIO_MODER_MODE10_Msk;
	GPIOA->AFR[1] = (GPIOA->AFR[1] & (~GPIO_AFRH_AFSEL10_Msk)) | (0x7 << GPIO_AFRH_AFSEL10_Pos);

	//UART configureren
	USART1->CR1 = 0;
	USART1->CR2 = 0;
	USART1->CR3 = 0;
	USART1->BRR = 417;
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

	while (1) {
	     if (tick == 1000){
			 // Start de ADC en wacht tot de sequentie klaar is
			 ADC1->CR |= ADC_CR_ADSTART;
			 while(!(ADC1->ISR & ADC_ISR_EOS));

			 // Lees de waarde in
			 value = ADC1->DR;
			 V = (value*3.0f)/4096.0f;
			 R = (10000.0f*V)/(3.0f-V);
			 temperatuur = ((1.0f/((logf(R/10000.0f)/3936.0f)+(1.0f/298.15f)))-273.15f);


			 printf("T=%.1f°C\r\n", temperatuur);
			 tick = 0;
	     }
	}

}
