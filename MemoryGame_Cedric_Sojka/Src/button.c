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

// Wird in der main.c definiert
extern void processPlayerInput(uint8_t bitEntered);

void initButtons(void) {
    // 1. Enable clocks for GPIOA
    RCC->AHB2ENR |= 1 << 0;

    // 2. PA0 und PA2 als Input konfigurieren
    GPIOA->MODER &= ~(3 << 0);
    GPIOA->MODER &= ~(3 << 4);

    // 3. Enable SYSCFG clock
    RCC->APB2ENR |= 1 << 0;

    // 4. Disable PA0 und PA2 pull-up/pull-down (Wie in den Folien)
    // Achtung: Maske ist 3 (zwei Bits pro Pin)!
    GPIOA->PUPDR &= ~(3 << 0);
    GPIOA->PUPDR &= ~(3 << 4);

    // 5. Select Port A as interrupt source (KORREKTUR: Beide liegen in EXTICR[0])
    SYSCFG->EXTICR[0] &= ~(0xF << 0); // PA0 auf EXTI0 mappen
    SYSCFG->EXTICR[0] &= ~(0xF << 8); // PA2 auf EXTI2 mappen

    // 6. Enable EXTI lines
    EXTI->IMR1 |= 1 << 0;
    EXTI->IMR1 |= 1 << 2;

    // 7. Enable rising edge for trigger
    EXTI->RTSR1 |= 1 << 0;
    EXTI->RTSR1 |= 1 << 2;

    // 8. Interrupts im NVIC freischalten
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);
}

// PA0: long (1)
void EXTI0_IRQHandler(void) {
    // Prüfen, ob der Interrupt wirklich von EXTI0 kommt
    if (EXTI->PR1 & (1 << 0)) {
        // Clear interrupt flag (STM32 erfordert Zuweisung, nicht |=)
        EXTI->PR1 = (1 << 0);

        // Debouncing: Prüfen ob seit dem letzten Druck 50ms vergangen sind
        if ((TIM2->CNT - lastTouchTime_PA0) > 800) {
            lastTouchTime_PA0 = TIM2->CNT; // Zeitstempel merken

            // Wenn wir im richtigen Spielmodus sind, Eingabe "1" senden
            processPlayerInput(1);
        }
    }
}

// PA2: short (0)
void EXTI2_IRQHandler(void) {
    // Prüfen, ob der Interrupt wirklich von EXTI2 kommt
    if (EXTI->PR1 & (1 << 2)) {
        // Clear interrupt flag
        EXTI->PR1 = (1 << 2);

        // Debouncing: 50ms
        if ((TIM2->CNT - lastTouchTime_PA2) > 800) {
            lastTouchTime_PA2 = TIM2->CNT; // Zeitstempel merken

            // Wenn wir im richtigen Spielmodus sind, Eingabe "0" senden
            processPlayerInput(0);
        }
    }
}
