#include "stm32g431xx.h"

#define DUTYCYCLE 20000

volatile int direction = 1;

void init(void)
{
  // PortB Initializiation
  RCC->CR |= 1<<24;             // Enable RCC clock
  RCC->AHB2ENR |= 1<<1;         // Enable clocks for GPIOB

  GPIOB->AFR[1] &= ~(0xF<<0);   // Clear alternate function bits for PB.8
  GPIOB->AFR[1] |= 1<<0;        // Set PB.8 as alternate function AF1
  GPIOB->MODER &= ~(3<<16);     // Clear mode bits for PB.8
  GPIOB->MODER |= 2<<16;        // Set PB.8 as alternate function mode

  // Timer16 Initializiation for PWM
  RCC->APB2ENR |= 1<<17;        // Enable clocks for TIM16

  TIM16->CR1 &= ~(1<<0);        // Disable TIM16 counter during configuration
  TIM16->PSC = 0;               // No prescaler: PWM frequency high enough
  TIM16->ARR = DUTYCYCLE;       // PWM period
  TIM16->CCR1 = 0;              // Duty cycle start value

  TIM16->CCMR1 &= ~(7<<4);      // PWM-Polarity Mode (High/Low): Clear OC1M bits
  TIM16->CCMR1 |= 6<<4;         // PWM mode 1 (OCM = Output Compare 1 Mode)
  TIM16->CCMR1 |= 1<<3;         // Enable preload for CCR1
  TIM16->CCER &= ~(1<<1);       // Active high polarity
  TIM16->CCER |= 1<<0;          // Enable channel 1 output to output pin

  TIM16->BDTR |= 1<<15;         // Set main output enable (MOE)
  TIM16->CR1 |= 1<<0;           // Enable TIM16 counter

  // Timer3 Initializiation for brightness update interrupt
  RCC->APB1ENR1 |= 1<<1;        // Enable clocks for TIM3

  TIM3->CR1 &= ~(1<<0);         // Disable TIM3 counter during configuration
  TIM3->PSC = 15999;            // 16 MHz / 16000 = 1 kHz
  TIM3->ARR = 10-1;             // Interrupt every 10 ms
  TIM3->CNT = 0;                // Reset counter

  TIM3->DIER |= 1<<0;           // Enable update interrupt
  TIM3->SR &= ~(1<<0);          // Clear update interrupt flag
  NVIC_EnableIRQ(TIM3_IRQn);    // Enable TIM3 interrupt vector in NVIC
  TIM3->CR1 |= 1<<0;            // Enable TIM3 counter
  __enable_irq();               // Enable *all* interrupts globally
}

int main(void)
{
  init();

  while (1)                     // Endless loop: CPU does nothing
  {}							// Brightness control is done in TIM3 interrupt
  return (1);
}

void TIM3_IRQHandler(void)
{
  if ((TIM3->SR & (1<<0)) != 0) // Test for update interrupt
  {
    TIM3->SR &= ~(1<<0);        // Clear update interrupt flag
    if (direction == 1)
    {
      if (TIM16->CCR1 < DUTYCYCLE)
      {
        TIM16->CCR1 += 100;     // Increase brightness
      }
      else
      {
        direction = 0;           // Change direction
      }
    }
    else
    {
      if (TIM16->CCR1 > 100)
      {
        TIM16->CCR1 -= 100;     // Decrease brightness
      }
      else
      {
        TIM16->CCR1 = 0;        // LED off
        direction = 1;          // Change direction
      }
    }
  }
}










/*
Damit wird die LED-Helligkeit automatisch im Hintergrund verändert.

Die relevante Zeitbasis ist hier:

TIM3->PSC = 15999;
TIM3->ARR = 10-1;

Bei 16 MHz Timer-Takt ergibt das:

16 MHz / 16000 = 1 kHz
1 Zählschritt = 1 ms
10 Zählschritte = 10 ms

Alle 10 ms wird also TIM3_IRQHandler() aufgerufen und der PWM-Wert TIM16->CCR1 verändert. Die Hauptschleife muss nichts mehr tun.

Die Helligkeitsänderung soll nicht mehr in der while(1)-Schleife verzögert werden, sondern zyklisch im Timer-Interrupt erfolgen.

Die Struktur ist dann:

TIM16 erzeugt das PWM-Signal für PB.8.
TIM3 erzeugt alle 10 ms einen Interrupt.
Im TIM3_IRQHandler wird TIM16->CCR1 schrittweise erhöht oder verringert.
Die while(1)-Schleife bleibt leer.

*/
