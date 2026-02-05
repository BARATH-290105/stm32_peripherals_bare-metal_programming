#include "stm32f103xb.h"
typedef enum
{
	IDLE=0,
	SAMPLE,
	PROCESS,
	TRANSMIT,
	ERROR
}ADC_STATES;

volatile uint32_t time_base = 0;
volatile uint32_t voltage = 0;
volatile uint8_t process_req = 0;
volatile uint8_t sample_req = 0;
volatile uint8_t transmit_done = 0;

volatile ADC_STATES ADC1_STATE=IDLE;

void SysClk_config(void)
{
	RCC->CFGR = 0;
}

void timer_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	TIM2->PSC = 7;
	TIM2->ARR = 999;
	TIM2->CR1 |= TIM_CR1_ARPE;

	TIM2->CR2 &= ~TIM_CR2_MMS;
	TIM2->CR2 |= (0b010 << 4);

	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_SetPriority(TIM2_IRQn,0);
	NVIC_EnableIRQ(TIM2_IRQn);
	TIM2->SR &= ~TIM_SR_UIF;

	TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN;

}

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		sample_req = 1;
		time_base++;
	}
}

void adc_init(void)
{
	RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN|RCC_APB2ENR_ADC1EN);
	RCC->CFGR &= ~RCC_CFGR_ADCPRE;
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2;

	GPIOA->CRL &= ~(0xF<<0);

	ADC1->SMPR2 &= ~(0b111<<0);
	ADC1->SMPR2 |= (0b111<<0);
	ADC1->SQR3 = 0;

	ADC1->CR1 |= ADC_CR1_EOCIE;
	NVIC_SetPriority(ADC1_IRQn,0);
	NVIC_EnableIRQ(ADC1_IRQn);

	ADC1->CR2 |= ADC_CR2_EXTTRIG;
	ADC1->CR2 &= ~ADC_CR2_EXTSEL;
	ADC1->CR2 |= (0x3<<17);
	ADC1->CR2 |= ADC_CR2_RSTCAL;
	ADC1->CR2 &= ~ADC_CR2_CONT;

	ADC1->CR2 |= ADC_CR2_ADON;
	while(ADC1->CR2 & ADC_CR2_RSTCAL);
	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);

}

void ADC1_IRQHandler(void)
{
	if(ADC1->SR & ADC_SR_EOC)
	{
		voltage = ADC1->DR;
		process_req = 1;
	}
}

void uart_init(void)
{
	RCC->APB2ENR |= (RCC_APB2ENR_USART1|RCC_APB2ENR_IOPAEN|RCC_APB2ENR_AFIOEN);

	GPIOA->CRH &= ~(0xFF<<(4*(9-8)));
	GPIOA->CRH |= (0xBB<<(4*(9-8)));

	USART1->BRR = (833<<4|5);

	USART1->CR1 |= USART_CR1_TXEIE;

	USART1->CR1 |= USART_CR1_TE;

	USART1->CR1 |= USART_CR1_UE;
}

void uart_ch(char ch)
{
	while(USART1->SR & USART_SR_TXE);
	USART1->DR = ch;
	transmit_done = 1;
}
void transmit(uint32_t voltage)
{
	uart_ch((voltage/1000)+48);
	uart_ch('.')
	uart_ch(((voltage/100)%10)+48);
	uart_ch(((voltage/10)%10)+48);
	uart_ch('\r');
	uart_ch('\n');

}
void ADC_FSM(void)
{
	switch(ADC1_STATE)
	{
	case IDLE:
		if(sample_req)
				{
					sample_req=0;
					ADC1_STATE = SAMPLE;
				}
		break;
	case SAMPLE:
		if(process_req)
		{
			process_req=0;
			ADC1_STATE = PROCESS;
		}
		break;
	case PROCESS:
		voltage = (voltage * 3300) / 4095;
		ADC1_STATE = TRANSMIT;
		break;
	case TRANSMIT:
		transmit(voltage);
		if(transmit_done)
			{transmit_done =0;
			ADC1_STATE = IDLE;}
		break;
	case ERROR:
		break;
	}
}

int main(void)
{
	SysClk_config();
	adc_init();
	timer_init();

	while(1)
	{
		ADC_FSM();
	}



}
