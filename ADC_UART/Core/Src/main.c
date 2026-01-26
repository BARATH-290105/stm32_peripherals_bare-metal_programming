#include "stm32f103xb.h"

volatile uint32_t time_base=0;
volatile uint32_t voltage=0;

void sysclk_config(void)
{
	RCC->CFGR = 0;
}

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		time_base++;
	}
}
void ADC1_IRQHandler(void)
{
	if(ADC1->SR & ADC_SR_EOC)
	{
		voltage = ADC1->DR;
	}
}
void timer_config(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	TIM2->PSC = 7;
	TIM2->ARR = 999;
	TIM2->CR1 |= TIM_CR1_ARPE;
	TIM2->CR2 &= ~TIM_CR2_MMS;
	TIM2->CR2 |= (0B010 << 4);
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_SetPriority(TIM2_IRQn,0);
	NVIC_EnableIRQ(TIM2_IRQn);
	TIM2->SR &= ~TIM_SR_UIF;
	TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN;

}

void adc_config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	GPIOA->CRL &= ~(0xF<<4);

	RCC->CFGR &= ~RCC_CFGR_ADCPRE;
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2;

	ADC1->SMPR2 &= ~ADC_SMPR2_SMP1;
	ADC1->SMPR2 |= (ADC_SMPR2_SMP1_2|ADC_SMPR2_SMP1_0);
	ADC1->SQR1=0;
	ADC1->SQR3=1;

	ADC1->CR1 |= ADC_CR1_EOCIE;
	NVIC_SetPriority(ADC1_IRQn,1);
	NVIC_EnableIRQ(ADC1_IRQn);

	ADC1->CR2 |= ADC_CR2_EXTTRIG;
	ADC1->CR2 &= ~ADC_CR2_EXTSEL;
	ADC1->CR2 |= (0x3<<17);

	ADC1->CR2 |= ADC_CR2_ADON;

	ADC1->CR2 |= ADC_CR2_RSTCAL;
	while(ADC1->CR2 & ADC_CR2_RSTCAL);

	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);

	ADC1->CR2 |= ADC_CR2_ADON;

}
void uart_config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	GPIOA->CRH &= ~(0xF<<4);
	GPIOA->CRH |= (0xB<<4);

	USART1->BRR = 8000000/115200;
	USART1->CR1 |= USART_CR1_TE;
	USART1->CR1 |= USART_CR1_UE;


}

void ch_tx(char ch)
{
	while((USART1->SR & USART_SR_TXE) == 0);
	USART1->DR = ch;
}

void str_tx(char *str)
{
	while(*str)
	{
		ch_tx(*str++);
	}
}

void send_voltage(uint32_t value)
{
	value = (value * 3300) /4095;

	ch_tx((value/1000)+'0');
	ch_tx('.');
	ch_tx(((value/100)%10)+'0');
	ch_tx(((value/10)%10)+'0');
	ch_tx((value%10)+'0');
}
int main(void)
{
	sysclk_config();
	timer_config();
	adc_config();
	uart_config();

	while(1)
	{
		str_tx("VOLTAGE = ");

		send_voltage(voltage);
		ch_tx('\r');
		ch_tx('\n');

		for(uint32_t i=0;i<500000;i++);
	}
}
