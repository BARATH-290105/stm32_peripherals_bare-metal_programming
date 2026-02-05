#include "stm32f103xb.h"

#define CMD_SIZE 5

volatile uint8_t new_cmd = 0;
volatile char ch = 0;

/* ---------- CLOCK ---------- */
void SysClk_config(void)
{
    RCC->CFGR = 0;
}

/* ---------- LED on PC13 (ACTIVE-LOW) ---------- */
void led_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    /* PC13 output push-pull, 2 MHz */
    GPIOC->CRH &= ~(0xF << (4 * (13 - 8)));
    GPIOC->CRH |=  (0x2 << (4 * (13 - 8)));

    /* LED OFF (PC13 HIGH) */
    GPIOC->BSRR = (1 << 13);
}

/* ---------- USART ---------- */
void uart_init(void)
{
    RCC->APB2ENR |= (RCC_APB2ENR_USART1EN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN);

    /* PA10 RX input floating */
    GPIOA->CRH &= ~(0xF << (4 * (10 - 8)));
    GPIOA->CRH |=  (0x4 << (4 * (10 - 8)));

    /* PA9 TX AF push-pull */
    GPIOA->CRH &= ~(0xF << (4 * (9 - 8)));
    GPIOA->CRH |=  (0xB << (4 * (9 - 8)));

    USART1->BRR = 8000000/115200;  // 8 MHz, 115200 baud
    USART1->CR1 |= (USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE);
    USART1->CR1 |= USART_CR1_UE;

    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
}

/* ---------- LED CONTROL ---------- */
void control_led(char ch)
{
    if (ch == 'O')
    {
        GPIOC->BRR = (1 << 13);     // LED ON (LOW)
    }
    else if (ch == 'N')
    {
        GPIOC->BSRR = (1 << 13);    // LED OFF (HIGH)
    }
}

/* ---------- USART ISR ---------- */
void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE)
    {
        (void)USART1->DR;
        GPIOC->BRR = (1 << 13);   // turn LED ON on ANY RX
    }
}


/* ---------- MAIN ---------- */
int main(void)
{
    SysClk_config();
    uart_init();
    led_init();

    while (1)
    {
        if (new_cmd)
        {
            new_cmd = 0;
            control_led(ch);
        }
        __WFI();
    }
}
