//#include "stm32g431xx.h"
//
//void systickDelayMs(int n)
//{
//  SysTick->LOAD = 4000;         // Load clockcycless per millisecond
//  SysTick->VAL = 0;             // Clear current counting register
//  SysTick->CTRL = 0x5;          // Enable Systick
//
//  for(int i =0;i<n;i++)
//    {                           // Wait for 1ms until the COUNT flag is set
//      while((SysTick->CTRL & 0x10000) ==0){}
//    }
//  SysTick->CTRL =0;             // Disable Systick
//}
//
//void init(void)
//{
//  // Port Initializiation
//  RCC->CR |= 1<<24;             // Bit24: Enable RCC clock
//  RCC->AHB2ENR |= 1<<1; 	// Bit1: Enable clocks for GPIOB
//  GPIOB->MODER = 0x00010000; 	// Bit8: Set PB8 (LED) as digital output
//
//  // ADC Initializiation: Input Port-Pin
//  RCC->AHB2ENR |= 1<<0; 	// Bit1: Enable clocks for GPIOA
//  GPIOA->MODER |= 0xC0; 	// Bit8: Set PA3 to analog mode
//
//  // ADC Initializiation: Conversion
//  RCC->AHB2ENR |= 1<<13; 	// Bit13: Enable clocks for ADC1 & ADC2
//  ADC12_COMMON->CCR |= 1<<17;   // Bit17: hclk/2 clock mode for ADC1 & ADC2
//
//  ADC1->CR &= ~(1<<29);         // Disable ADC1 deep power down mode
//  ADC1->SQR1 = 0x0100;          // First conversion ADC1_IN4 == GPIOA.3
//  ADC1->CR |= 1<<28;            // Enable voltage regulator
//  systickDelayMs(1);            // Wait 1 ms to start-up voltage regulator
//
//  ADC1->CR |= 1<<0;             // Enable ADC1
//}
//
//void main(void)
//{
//  long int rand = 0;
//  unsigned int LSBit = 0;
//  unsigned int i = 0;
//
//  init();
//  systickDelayMs(10);                   // Wait 10 ms to stabilize ADC
//
//  while (1)                             // Endless loop
//  {
//    rand = 0;                           // Clear variable rand
//    for (i = 0; i<32; i++)
//    {
//      ADC1->CR |= 1<<2;                 // Start analog-digital conversion
//      while (!(ADC1->ISR & 0x04)){}     // End of conversion reached?
//      LSBit = ADC1->DR & 0x01;          // Grab LSB of conversion
//      LSBit = LSBit << i;               // Shift LSB (only one Bit) to Bit i
//      rand = rand | LSBit;              // Insert it at the right place
//    }
//  }
//}
//
//
//
//
//
//
//
//
