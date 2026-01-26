#include "stm32f103xb.h"

volatile uint32_t base=0;

void SysClock_config(void)
{
	RCC->CR |= RCC_CR_HSION;
	while((RCC->CR & RCC_CR_HSIRDY)!=RCC_CR_HSIRDY);

	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_HSI;
	while((RCC->CFGR & RCC_CFGR_SWS) != 0);

}

void led_config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	GPIOA->CRL &= ~(0xFFF<<0);
	GPIOA->CRL |= (0x222<<0);

}

void time_base_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = 7;
	TIM2->ARR = 999;
	TIM2->CR1 |= TIM_CR1_ARPE;
	TIM2->CR1 &= ~TIM_CR1_CEN;
	TIM2->CNT = 0;
	TIM2->SR  = 0;

	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_SetPriority(TIM2_IRQn,0);
	NVIC_EnableIRQ(TIM2_IRQn);

	TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		base++;
	}
}

int main(void)
{
	SysClock_config();
	led_config();
	time_base_init();
	uint32_t last_2s=0, last_4s=0,last_6s=0;

	while(1)
	{
		if((base-last_2s)>=2000)
		{
			last_2s += 2000;
			GPIOA->ODR ^= (1<<0);
		}
		if((base-last_4s)>=4000)
		{
			last_4s += 4000;
			GPIOA->ODR ^= (1<<1);
		}
		if((base-last_6s)>=6000)
		{
			last_6s += 6000;
			GPIOA->ODR ^= (1<<2);
		}

	}


}
