
/*
 * button.c
 *
 *  Created on: 20.06.2026
 *      Author: sojka
 */

#include "stm32g431xx.h"
#include "button.h"

// Zeitstempel für das sichere Debouncing
volatile uint32_t lastTouchTime_PA0 = 0;
volatile uint32_t lastTouchTime_PA2 = 0;

// Externe Funktion zur Verarbeitung der Spielereingabe
extern void processPlayerInput(uint8_t bitEntered);

void initButtons(void) {
	// Takt für GPIOA aktivieren
    RCC->AHB2ENR |= 1 << 0;

    // PA0 und PA2 als Eingang konfigurieren
    GPIOA->MODER &= ~(3 << 0);
    GPIOA->MODER &= ~(3 << 4);

    // Takt für SYSCFG aktivieren
    RCC->APB2ENR |= 1 << 0;

    // Pull-Up/Pull-Down-Widerstände für PA0 und PA2 deaktivieren
    GPIOA->PUPDR &= ~(3 << 0);
    GPIOA->PUPDR &= ~(3 << 4);

    // Port A (PA0 und PA2) als Interrupt-Quelle konfigurieren
    SYSCFG->EXTICR[0] &= ~(0xF << 0); // PA0 auf EXTI0 mappen
    SYSCFG->EXTICR[0] &= ~(0xF << 8); // PA2 auf EXTI2 mappen

    // EXTI-Leitungen für Line 0 und 2 aktivieren
    EXTI->IMR1 |= 1 << 0;
    EXTI->IMR1 |= 1 << 2;

    // Trigger auf steigende Flanke setzen
    EXTI->RTSR1 |= 1 << 0;
    EXTI->RTSR1 |= 1 << 2;

    // Interrupts im NVIC aktivieren
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);
}

// Interrupt-Handler für PA0 (Eingabe: 1)
void EXTI0_IRQHandler(void) {
    // Interrupt-Flag für EXTI0 prüfen
    if (EXTI->PR1 & (1 << 0)) {
    	// Interrupt-Flag löschen
        EXTI->PR1 = (1 << 0);

        // Software-Entprellung
        if ((TIM2->CNT - lastTouchTime_PA0) > 800) {
            lastTouchTime_PA0 = TIM2->CNT;

            // Eingabe verarbeiten
            processPlayerInput(1);

            // Zeitstempel nach der Verarbeitung aktualisieren
            lastTouchTime_PA0 = TIM2->CNT;

            // Etwaige während der Verarbeitung gesetzte Interrupt-Flags löschen
            EXTI->PR1 = (1 << 0);
        }
    }
}

// Interrupt-Handler für PA2 (Eingabe: 0)
void EXTI2_IRQHandler(void) {
	// Interrupt-Flag für EXTI2 prüfen
    if (EXTI->PR1 & (1 << 2)) {
    	// Interrupt-Flag löschen
        EXTI->PR1 = (1 << 2);

        // Software-Entprellung
        if ((TIM2->CNT - lastTouchTime_PA2) > 800) {
            lastTouchTime_PA2 = TIM2->CNT; // Zeitstempel merken

            // Eingabe verarbeiten
            processPlayerInput(0);

            // Zeitstempel nach der Verarbeitung aktualisieren
			lastTouchTime_PA2 = TIM2->CNT;

			// Etwaige während der Verarbeitung gesetzte Interrupt-Flags löschen
			EXTI->PR1 = (1 << 2);
        }
    }
}
