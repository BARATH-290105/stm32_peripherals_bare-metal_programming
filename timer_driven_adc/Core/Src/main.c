#include "stm32f103xb.h"

volatile uint32_t TimeBase=0;
volatile uint16_t voltage=0;

void SysClock_config(void)
{
	RCC->CFGR = 0;
}

void TimeBase_config(void)
{

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

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
	TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		TimeBase++;
	}
}

void Adc_config()
{
	RCC->APB2ENR |=  (RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPAEN);

	RCC->CFGR &= ~RCC_CFGR_ADCPRE;
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2;

	GPIOA->CRL &= ~(0xF<<4);

	ADC1->CR1 |= ADC_CR1_EOCIE;

	ADC1->CR2 |= ADC_CR2_EXTTRIG;
	ADC1->CR2 &= ~ADC_CR2_EXTSEL;
	ADC1->CR2 |= (0x3<<17);

	ADC1->SMPR2 &= ~ADC_SMPR2_SMP1;
	ADC1->SMPR2 |= (ADC_SMPR2_SMP1_0|ADC_SMPR2_SMP1_2);

	ADC1->SQR1 = 0;
	ADC1->SQR3 = 1;

	NVIC_SetPriority(ADC1_IRQn,1);
	NVIC_EnableIRQ(ADC1_IRQn);

	ADC1->CR2 |= ADC_CR2_ADON;

	ADC1->CR2 |= ADC_CR2_RSTCAL;
	while(ADC1->CR2 & ADC_CR2_RSTCAL);

	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);

	ADC1->CR2 |= ADC_CR2_ADON;
}

void ADC1_IRQHandler(void)
{
	if(ADC1->SR & ADC_SR_EOC)
	{
		voltage = ADC1->DR;
	}
}


int main(void)
{
	SysClock_config();
	TimeBase_config();
	Adc_config();

	while(1)
	{

	}
}
