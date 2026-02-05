#include "stm32f103xb.h"

typedef enum
{
	LED_OFF=0,
	LED_ON
}LED_STATE;

volatile LED_STATE LED_ONE;
volatile uint32_t time_base = 0;
volatile uint8_t TOGGLE_LED_ONE = 0;
uint32_t led_one_base = 0;
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


	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_SetPriority(TIM2_IRQn,0);
	NVIC_EnableIRQ(TIM2_IRQn);

	TIM2->SR &= ~TIM_SR_UIF;
	TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN;

}

void led_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	GPIOA->CRL &= ~(0xF << 0 );
	GPIOA->CRL |= (0x2 << 0);
	GPIOA->BSRR = (0x1<<(16+0));
}

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		time_base++;
	}
}

void LED_FSM(void)
{
	if(TOGGLE_LED_ONE)
	{
		TOGGLE_LED_ONE = 0;
		switch(LED_ONE)
		{
		case LED_ON:
			GPIOA->BSRR = (0x1<<(0+16));
			LED_ONE = LED_OFF;
			break;

		case LED_OFF:
			GPIOA->BSRR = (0x1<<(0));
			LED_ONE = LED_ON;
			break;
		}

	}
}

int main(void)
{


	SysClk_config();
	timer_init();
	led_init();

	while(1)
	{
		if((time_base - led_one_base) >= 2000)
		{
			led_one_base += 2000;
			TOGGLE_LED_ONE = 1;

		}
		LED_FSM();
	}
}
