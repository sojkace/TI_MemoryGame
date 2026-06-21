#include "stm32g431xx.h"
#include "tools.h"

/* --- INITIALISIERUNG DES HARDWARE-TIMERS (TIM2) --- */
void initHardwareDelay(void) {
    // 1. Takt für Timer 2 (TIM2) auf dem APB1-Bus aktivieren (Bit 0)
    RCC->APB1ENR1 |= (1 << 0);

    // 2. Prescaler (Vorteiler) einstellen.
    // Ziel: Der Timer soll exakt jede Millisekunde (1 kHz) um 1 hochzählen.
    // Bei einem Standard-Systemtakt von 16 MHz (HSI): 16.000.000 Hz / 16.000 = 1.000 Hz.
    // Da die Hardware immer (Wert + 1) rechnet, tragen wir (16000 - 1) ein.
    TIM2->PSC = 16000 - 1;

    // 3. Auto-Reload-Register (ARR) auf den maximalen Wert setzen.
    // TIM2 ist ein 32-Bit-Timer. Mit 0xFFFFFFFF läuft er über 49 Tage, bevor er überläuft.
    // Dies garantiert eine fortlaufende, unterbrechungsfreie Systemzeit für das Debouncing.
    TIM2->ARR = 0xFFFFFFFF;

    // 4. Update-Generation (UG) erzwingen.
    // Dies stellt sicher, dass der Timer den neuen Prescaler-Wert sofort übernimmt
    // und nicht erst nach einem eventuellen Überlauf des alten Wertes.
    TIM2->EGR |= (1 << 0);

    // 5. Timer 2 endgültig starten (CEN-Bit setzen)
    TIM2->CR1 |= (1 << 0);
}

/* --- NICHT-DESTRUKTIVE VERZÖGERUNGSFUNKTION --- */
void timerDelayMs(uint32_t ms) {
    // 1. Aktuelle "Systemzeit" sichern, OHNE das Timer-Register (CNT) zu verändern.
    // Dies ist essenziell, um das Debouncing in anderen Interrupts nicht zu zerstören.
    uint32_t startTime = TIM2->CNT;

    // 2. Aktives Warten (Polling), bis die geforderte Zeitdifferenz erreicht ist.
    // Durch die unsigned 32-Bit-Arithmetik wird ein eventueller Timer-Überlauf
    // nach 49 Tagen automatisch korrekt gehandhabt.
    while ((TIM2->CNT - startTime) < ms) {
        // Leere Schleife zum Halten der Ausführung
    }
}
