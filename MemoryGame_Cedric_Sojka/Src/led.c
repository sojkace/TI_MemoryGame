/*
 * led.c
 *
 *  Created on: 19.06.2026
 *      Author: sojka
 */

#include "stm32g431xx.h"
#include "led.h"
#include "tools.h"

void initLEDs(void) {
    // 1. GPIO B Takt aktivieren
    RCC->AHB2ENR |= (1 << 1);

    // 2. Pin PB8 als digitalen Ausgang setzen (01 an Bitposition 16/17)
    GPIOB->MODER &= ~(3 << 16); // Erst sauber löschen
    GPIOB->MODER |=  (1 << 16); // Dann auf Output setzen

    // (Die Timer-3-Interrupt-Konfiguration aus deinem Testcode kommt später
    // hier hinein, wenn wir die led_playSequence() ausprogrammieren)
}

void led_blinkSuccess(void) {
    // Statechart sagt: LED 2x lang blinken
    for(int i = 0; i < 2; i++) {
        GPIOB->ODR |= (1 << 8);  // LED an
        timerDelayMs(150);     // lang warten
        GPIOB->ODR &= ~(1 << 8); // LED aus
        timerDelayMs(150);     // kurz Pause
    }
}

void led_blinkGameOver(void) {
    // Statechart sagt: LED 3x schnell blinken
    for(int i = 0; i < 3; i++) {
        GPIOB->ODR |= (1 << 8);
        timerDelayMs(150);     // kurz warten
        GPIOB->ODR &= ~(1 << 8);
        timerDelayMs(150);
    }
}

void led_inputBlink(void) {
	GPIOB->ODR |= (1 << 8);  // LED an
	timerDelayMs(150);     // lang warten
	GPIOB->ODR &= ~(1 << 8); // LED aus
	timerDelayMs(150);
}

void led_playSequence(uint32_t sequence, uint8_t level) {
	// level 1 startet bei 5 bits
	level = level + 5;
	// Gehe alle Bits bis zum aktuellen Level durch
	    for(int i = 0; i < (level - 1); i++) {

	        // Aktuelles Bit herausfiltern (0 oder 1)
	        uint8_t currentBit = (sequence >> i) & 0x01;

	        GPIOB->ODR |= (1 << 8); // LED an

	        // Je nachdem, ob es eine 1 oder 0 ist, warten wir unterschiedlich lange
	        if (currentBit == 1) {
	        	timerDelayMs(1000); // 1 = langes Leuchten
	        } else {
	        	timerDelayMs(350); // 0 = kurzes Leuchten
	        }

	        GPIOB->ODR &= ~(1 << 8); // LED wieder aus
	        timerDelayMs(1000);     // Pause, bevor das nächste Bit kommt
	    }
}


