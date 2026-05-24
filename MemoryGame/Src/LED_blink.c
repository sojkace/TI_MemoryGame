/*
 * LED_blink.c
 *
 *  Created on: 25.05.2026
 *      Author: sojka
 */

#include "LED_blink.h"

void SystemInit(void) {
	RCC->CR |= 1<<24;
	RCC->AHB2ENR |= 1<<1;
	GPIOB->MODER = 0x00010000;
}

void delay(void) {
	long int i;
	for (i=0; i < 400000; i++);
}

void startBlinken(void) {
	SystemInit();
	while (1) {
		GPIOB->ODR |= 1<<8;
		delay();
		GPIOB->ODR &= 0;
		delay();
	}
}
