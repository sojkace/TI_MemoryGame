/*
 * tools.c
 *
 *  Created on: 19.06.2026
 *      Author: sojka
 */
#include "stm32g431xx.h"
#include "tools.h"

void initHardwareDelay(void) {
    // 1. Takt für Timer 2 (TIM2) aktivieren (Bit 0 im APB1ENR1)
    RCC->APB1ENR1 |= (1 << 0);

    // 2. Prescaler (Vorteiler) einstellen.
    // Wir wollen, dass der Timer exakt jede Millisekunde (1 kHz) 1x hochzählt.
    // Wenn dein Board nach dem Reset mit dem Standard-Takt von 16 MHz (HSI) läuft:
    // 16.000.000 Hz / 16.000 = 1.000 Hz (1 ms pro Tick).
    // Da der Prescaler immer (Wert + 1) rechnet, tragen wir 16000 - 1 ein:
    TIM2->PSC = 16000 - 1;

    /* HINWEIS: Falls du den Systemtakt später über die PLL auf die maximalen
     * 170 MHz des STM32G4 hochdrehst, musst du diesen Wert anpassen zu:
     * TIM2->PSC = 170000 - 1;
     */

    // 3. ARR auf maximalen Wert setzen (32-Bit Timer -> 0xFFFFFFFF)
    // Er zählt also fast ewig, bevor er wieder auf 0 springt.
    TIM2->ARR = 0xFFFFFFFF;

    // 4. WICHTIG: Update sofort erzwingen! (Das hat gefehlt)
	TIM2->EGR |= (1 << 0);

    // 4. Timer starten (CEN-Bit im CR1 setzen)
    TIM2->CR1 |= (1 << 0);
}

void timerDelayMs(uint32_t ms) {
    // Wir nullen den Zähler des Timers beim Aufruf der Funktion
    TIM2->CNT = 0;

    // Wir blockieren hier einfach so lange, bis der Timer die gewünschte
    // Anzahl an Millisekunden hochgezählt hat. Keine Bitmasken nötig!
    while (TIM2->CNT < ms) {
        // Warten...
    }
}

