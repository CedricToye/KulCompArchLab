#include <stdint.h>
#include <stm32l4xx.h>
// Cédric Toye

int mux = 0;
int i = 0;
float waarde;
int angle = 0;

void delay(unsigned int n) {
	volatile unsigned int delay = n;
	while (delay--);
}

void read_I2C(int reg){
	while((I2C1->ISR & I2C_ISR_BUSY));
	I2C1->CR2 &= ~(1<<10); //enable write mode
	I2C1->CR2 &= ~I2C_CR2_AUTOEND_Msk;
	I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
	I2C1->CR2 |= I2C_CR2_NACK_Msk;
	I2C1->CR2 |=  (1 << 13)|(1 << 16)|(0x53 << 1);

	while(((I2C1->ISR & (1<<4)) == 0) && ((I2C1->ISR & (1<<1)) == 0)){}

	if((I2C1->ISR & (1<<4)) != 0){
		return;
	}

    I2C1->TXDR = reg; //register doorsturen
    while((I2C1->ISR & (1<<6)) == 0);

    I2C1->CR2 |= I2C_CR2_AUTOEND_Msk;
    I2C1->CR2 |= (1<<10); //enable read mode

    //read
    I2C1->CR2 |=  (1 << 16)|(0x53 << 1);
    I2C1->CR2 |= (1<<13);
    while(!(I2C1->ISR & I2C_ISR_RXNE));

    return I2C1->RXDR;
}

void write_I2C(void){
    I2C1->CR2 &= ~(1<<10);//enable write mode
	I2C1->CR2 |= I2C_CR2_NACK_Msk;
    I2C1->CR2 |=  (1 << 13)|(2 << 16)|(0x53 << 1);
    while((I2C1->ISR & (1<<4)) == 0 && (I2C1->ISR & (1<<1)) == 0);
    if((I2C1->ISR & (1<<4)) != 0){
        return;
    }

    I2C1->TXDR = reg; //register doorsturen

    while(I2C1->ISR & (1<<4) == 0 && I2C1->ISR & (1<<1) == 0);
    if((I2C1->ISR & (1<<4)) != 0){
        return;
    }
    I2C1->TXDR = data;


void SysTick_Handler(void) {
	switch (mux) {
	case 0:
		clear();
		GPIOA->ODR &= ~GPIO_ODR_OD8;
		GPIOA->ODR &= ~GPIO_ODR_OD15;
		segments( angle / 1000);
		break;
	case 1:
		clear();
		GPIOA->ODR |= GPIO_ODR_OD8;
		GPIOA->ODR &= ~GPIO_ODR_OD15;
		segments(( angle / 100) % 10);
		break;
	case 2:
		clear();
		GPIOA->ODR |= GPIO_ODR_OD6;
		GPIOA->ODR &= ~GPIO_ODR_OD8;
		GPIOA->ODR |= GPIO_ODR_OD15;
		segments(( angle % 100) / 10);
		break;
	case 3:
		clear();
		GPIOA->ODR |= GPIO_ODR_OD8;
		GPIOA->ODR |= GPIO_ODR_OD15;
		segments(( angle % 100) % 10);
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

int __io_putchar(int ch){
    while(!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = ch;
}

int main(void) {

	SysTick_Config(48000);

	NVIC_SetPriority(SysTick_IRQn, 128);
	NVIC_EnableIRQ(SysTick_IRQn);

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk; // Activating clock block A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk; // Activating clock block B
	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN; // Activating clock I2C

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

	volatile int16_t array[3];
	write_I2C(1<<3,0x2D);
	array[0] =  read_I2C(0x2D);

	while (1) {
		for (int i = 0; i<3; i++){
			array[i] = read_I2C(0x32+i*2)<<8+ read_I2C(0x32+i*2+1);
			}

		int xy = sqrt(array[0]^2+array[1]^2);
		int xyz = sqrt(xy^2+array[2]^2);
		printf("%2.2f",(acos(array[2]/(sqrt(array[0]*array[0]+array[1]*array[1]+array[2]*array[2]))))*(180/3.14));
		printf("\n\r");
		angle = (acos(array[2]/(sqrt(array[0]*array[0]+array[1]*array[1]+array[2]*array[2]))))*(180/3.14)*10;

		delay(5000);
	}
}


