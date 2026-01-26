#include "stm32f103xb.h"

volatile uint32_t base=0;

void time_base_init()
{
	TIM2->PSC = 7;                 //8MHz/(7+1)=1MHz clock frequency
	TIM2->ARR = 999;
	TIM2->CNT=0;//1ms before overflow
	TIM2->CR1 |= TIM_CR1_ARPE;      //enabling pre-load register for safe reload
	TIM2->SR &= ~TIM_SR_UIF;
	TIM2->CR1 |= TIM_CR1_CEN;
}

void time_base_poll()
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		base++;
	}
}

int main(void)
{

	uint32_t last_2s=0,last_5s=0;

	RCC->CR |= RCC_CR_HSION;
	while((RCC->CR & RCC_CR_HSIRDY)==0);

	RCC->CFGR &= ~RCC_CFGR_SW_HSI;
	while((RCC->CFGR & RCC_CFGR_SWS)!=RCC_CFGR_SWS_HSI);

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	GPIOA->CRL &= ~(0xFF);
	GPIOA->CRL |= (0x22<<0);            //A0 and A1 as output pins

	time_base_init();
	GPIOA->BSRR = (0x3<<16);

	while(1)
	{
		time_base_poll();

		if((base-last_2s) >= 2000)
		{
			last_2s+=2000;
			GPIOA->ODR ^= (1<<0);  //toggle every 2s
		}
		if((base-last_5s)>=5000)
		{
			last_5s+=5000;
			GPIOA->ODR ^= (1<<1);   //toggle every 5s
		}
	}
}
