#include "stm32f103xb.h"
#define BUFFER_SIZE 128

volatile uint32_t time_base=0;
volatile uint32_t voltage=0;
volatile uint16_t adc_buffer[BUFFER_SIZE];

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		time_base++;
	}
}

void sysclk_config(void)
{
	RCC->CFGR = 0;
}

void timer_config(void)
{
	RCC->APB1ENR |=RCC_APB1ENR_TIM2EN;

	TIM2->PSC = 7;
	TIM2->ARR = 999;
	TIM2->CR1 |= TIM_CR1_ARPE;
	TIM2->CR2 &= ~TIM_CR2_MMS;
	TIM2->CR2 |= (0b010<<4);

	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_SetPriority(TIM2_IRQn,0);
	NVIC_EnableIRQ(TIM2_IRQn);

	TIM2->SR &= ~TIM_SR_UIF;
	TIM2->CNT=0;
	TIM2->CR1|= TIM_CR1_CEN;
}

void adc_config(void)
{
	RCC->CFGR &= ~RCC_CFGR_ADCPRE;
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2;

	RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN|RCC_APB2ENR_ADC1EN|RCC_APB2ENR_AFIOEN);

	GPIOA->CRL &= ~(0xF<<4);

	ADC1->CR1=0;

	ADC1->CR2 |= ADC_CR2_EXTTRIG;
	ADC1->CR2 &= ~ADC_CR2_EXTSEL;
	ADC1->CR2 |= (0b011<<17);
	ADC1->CR2 |= ADC_CR2_DMA;

	ADC1->SMPR2 &= ~ADC_SMPR2_SMP1;
	ADC1->SMPR2 |= ADC_SMPR2_SMP1_2;
	ADC1->SQR1 = 0;
	ADC1->SQR3 = 1;

	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_RSTCAL;
	while(ADC1->CR2 & ADC_CR2_RSTCAL);
	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);

	ADC1->CR2 |= ADC_CR2_ADON;
}

void dma_config(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	DMA1_Channel1->CCR &= ~DMA_CCR_EN;

	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
	DMA1_Channel1->CMAR = (uint32_t)adc_buffer;
	DMA1_Channel1->CNDTR = BUFFER_SIZE;

	DMA1_Channel1->CCR |= (DMA_CCR_MINC|DMA_CCR_CIRC|DMA_CCR_PSIZE_0|DMA_CCR_MSIZE_0|DMA_CCR_PL_1);

	DMA1_Channel1->CCR |= DMA_CCR_EN;

}

void uart_config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN);
	GPIOA->CRH &= ~(0xF<<4);
	GPIOA->CRH |= (0b1011<<4);

	USART1->BRR = 8000000/115200;

	USART1->CR1 |= USART_CR1_TE;
	USART1->CR1 |= USART_CR1_UE;

}

void ch_tx(char ch)
{
	while((USART1->SR & USART_SR_TXE)==0);
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
	value = value*3300 / 4095;

	ch_tx((value/1000)+48);
	ch_tx('.');
	ch_tx(((value/100)%10)+48);
	ch_tx(((value/10)%10)+48);
	ch_tx((value%10)+48);
}
int main (void)
{
	uint16_t write_index=0,last_index=0;

	sysclk_config();
	timer_config();
	adc_config();
	dma_config();
	uart_config();

	while(1)
	{
		write_index = BUFFER_SIZE - DMA1_Channel1->CNDTR;

		if(write_index != last_index)
		{
			last_index = (write_index==0)?BUFFER_SIZE-1:write_index-1;
			voltage = adc_buffer[last_index];
		}

		str_tx("VOLTAGE = ");
		send_voltage(voltage);
		ch_tx('\r');
		ch_tx('\n');

		for(uint32_t i=0;i<500000;i++);
	}
}
