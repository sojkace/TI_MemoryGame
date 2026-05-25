/*

 LED_blink_timer.c

 Created on: 25.05.2026

 Author: sojka
 */

// Dies ist die zentrale Header-Datei, die alle spezifischen Speicheradressen für den STM32G431 enthält.
#include "stm32g431xx.h"

void initLEDBlinkTimer(void) {
	// Grundkonfiguration für den Port B wie in Variante 1
	RCC->CR |= 1 << 24;
	RCC->AHB2ENR |= 1 << 1;
	GPIOB->MODER = 0x00010000;

	// Aktiviert den Takt für Timer 3 (TIM3). Dieser hängt am APB1 (Advanced Peripheral Bus 1).
	// Durch Bit 1 im Register APB1ENR1 bekommt Timer 3 nun den Systemtakt.
	RCC->APB1ENR1 |= 1 << 1;

	// PSC = Prescaler (Vorteiler). Teilt die reinkommende Frequenz.
	// Der Teiler ist immer (Wert + 1). Hier also (99 + 1) = 100.
	// Wenn der Systemtakt z.B. 170 MHz ist, läuft der Timer jetzt mit 1,7 MHz.
	TIM3->PSC = 99;

	// ARR = Auto-Reload Register. Bestimmt, wie weit der Timer zählt.
	// Er zählt von 0 hoch bis 60000. Danach springt er sofort automatisch wieder auf 0 zurück.
	TIM3->ARR = 60000;

	// CR1 = Control Register 1. Bit 0 ist das CEN-Bit (Counter Enable).
	// Durch das Setzen dieses Bits fängt Timer 3 ab sofort im Hintergrund an zu zählen!
	TIM3->CR1 = 1 << 0;

}

void startBlinkTimer(void) {
	initLEDBlinkTimer(); // Initialisiert Port B und den Timer.

	while (1) { // Wieder eine Endlosschleife.

		// CNT = Counter Register. Hier steht der aktuelle Zählwert des Timers drin (Live-Wert).
		// Sobald der Timer beim Hochzählen exakt den Wert 40000 erreicht...
		if (TIM3->CNT > 40000 && TIM3->CNT < 50000) {
			GPIOB->ODR |= 1 << 8; // ...wird Pin PB8 auf HIGH gesetzt (LED an).
		}

		// Wenn der Timer weiterläuft und exakt 50000 erreicht...
		if (TIM3->CNT > 50000 && TIM3->CNT < 60000) {
			// ...wird versucht, den Pin wieder auf LOW zu setzen.
			// ACHTUNG: '0<<8' ergibt mathematisch einfach nur 0.
			// Der Code führt also eigentlich aus: GPIOB->ODR &= 0;
			// Auch hier wird wieder der gesamte Port B genullt.
			// Richtig wäre, um nur PB8 auszuschalten: GPIOB->ODR &= ~(1<<8);
			GPIOB->ODR &= 0 << 8;
		}

		// Danach läuft der Timer noch bis 60000 (ARR-Wert), springt auf 0 und das Spiel beginnt von vorn.
	}

}
