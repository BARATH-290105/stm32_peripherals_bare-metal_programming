#include "stm32f103xb.h"
#define LCD_ADDR 0x27
#define LCD_EN 0b00000100
#define LCD_RS 0b00000001
#define LCD_BL 0b00001000
#define LCD_RW 0b00000010
volatile uint32_t base_tick=0;

void sysclk_config(void)
{
	RCC->CFGR = 0;
}
void timebase_config(void)
{

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	TIM2->PSC = 7;
	TIM2->ARR = 999;
	TIM2->CR1 |= TIM_CR1_ARPE;

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
		base_tick++;
	}
}


void i2c_config(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

	GPIOB->CRL &= ~(0xF<<(4*6)|0xF<<(4*7));
	GPIOB->CRL |= (0xF<<(4*6)|0xF<<(4*7));

	I2C1->CR1 |= I2C_CR1_SWRST;
	I2C1->CR1 &= ~I2C_CR1_SWRST;

	I2C1->CR2 = 8;
	I2C1->CCR = 40;
	I2C1->TRISE = 9;

	I2C1->CR1 |= I2C_CR1_PE;
}


void i2c_write(uint8_t addr, uint8_t data)
{
	while(I2C1->SR2 & I2C_SR2_BUSY);

	I2C1->CR1 |= I2C_CR1_START;
	while((I2C1->SR1 & I2C_SR1_SB) == 0);

	I2C1->DR = addr<<1;
	while((I2C1->SR1 & I2C_SR1_ADDR) == 0);
	(void)I2C1->SR2;

	while((I2C1->SR1 & I2C_SR1_TXE) == 0);
	I2C1->DR = data;

	while((I2C1->SR1 & I2C_SR1_BTF) == 0);
	I2C1->CR1 |= I2C_CR1_STOP;

}

void lcd_tx(uint8_t byte)
{

	i2c_write(LCD_ADDR, byte|LCD_EN);

	for(uint16_t i=0;i<10000;i++);

	i2c_write(LCD_ADDR, byte);

}
void lcd_cmd(uint8_t cmd)
{
	uint8_t upper = (0xF0 & cmd)|LCD_BL;
	uint8_t lower = (cmd << 4)|LCD_BL;

	lcd_tx(upper);
	lcd_tx(lower);
}

void lcd_data(uint8_t data)
{
	uint8_t upper = (0xF0 & data)|LCD_BL|LCD_RS;
	uint8_t lower = (data << 4)|LCD_BL|LCD_RS;

	lcd_tx(upper);
	lcd_tx(lower);
}

void lcd_display(char *str)
{
	while(*str)
	{
		lcd_data(*str++);
	}
}
void lcd_config(void)
{
	for(uint32_t i=0;i<50000;i++);

	lcd_cmd(0x33);
	lcd_cmd(0x32);
	lcd_cmd(0x28);
	lcd_cmd(0x0C);
	lcd_cmd(0x06);
	lcd_cmd(0x01);

	for(uint16_t i=0;i<50000;i++);
}
int main(void)
{
	sysclk_config();
	timebase_config();
	i2c_config();
	lcd_config();


	lcd_display("EMBEDDED SYSTEM");
	while(1);

}
