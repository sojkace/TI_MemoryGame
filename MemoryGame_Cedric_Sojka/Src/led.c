#include "stm32g431xx.h"
#include "led.h"
#include "tools.h"

/* --- INITIALISIERUNG DER HARDWARE FÜR DIE LED --- */
void initLEDs(void) {
    // 1. Takt für GPIO Port B aktivieren (Bus AHB2)
    RCC->AHB2ENR |= (1 << 1);

    // 2. Pin PB8 als digitalen Ausgang konfigurieren (Bitmaske: 01)
    // Dieser Pin steuert die grüne User-LED auf dem STM32G431-Nucleo-Board
    GPIOB->MODER &= ~(3 << 16); // Modus-Register für Pin 8 (Bits 16 & 17) zurücksetzen
    GPIOB->MODER |=  (1 << 16); // Auf General Purpose Output Mode setzen
}

/* --- FEEDBACK-FUNKTIONEN (SPIELSTATUS) --- */

// Signalisiert den erfolgreichen Abschluss eines Levels (2x langes Blinken)
void led_blinkSuccess(void) {
    for(int i = 0; i < 2; i++) {
        GPIOB->ODR |= (1 << 8);  // LED einschalten
        timerDelayMs(150);       // Leuchtdauer
        GPIOB->ODR &= ~(1 << 8); // LED ausschalten
        timerDelayMs(150);       // Pausendauer
    }
}

// Signalisiert einen Fehler und den Reset des Spiels (3x kurzes Blinken)
void led_blinkGameOver(void) {
    for(int i = 0; i < 3; i++) {
        GPIOB->ODR |= (1 << 8);
        timerDelayMs(150);
        GPIOB->ODR &= ~(1 << 8);
        timerDelayMs(150);
    }
}

// Direkte optische Rückmeldung beim Betätigen der Touch-Pins durch den Spieler
void led_inputBlink(uint8_t bitEntered) {
    GPIOB->ODR |= (1 << 8); // LED sofort einschalten

    // Unterscheidung der Leuchtdauer basierend auf der Eingabe
    if (bitEntered == 1) {
        timerDelayMs(1000); // Langes Leuchten für logische "1"
    } else {
        timerDelayMs(350);  // Kurzes Leuchten für logische "0"
    }

    GPIOB->ODR &= ~(1 << 8); // LED nach Ablauf der Zeit ausschalten
}

/* --- ABSPIELEN DER ZUFALLS-SEQUENZ --- */
// Wandelt die LSBs des Random-Seeds in optische Signale um
void led_playSequence(uint32_t sequence, uint8_t level) {
    // Gemäß Anforderung: Level 1 beginnt mit 5 abzufragenden Bits
    level = level + 5;

    // Iteration über die geforderte Anzahl an Bits für das aktuelle Level
    for(int i = 0; i < (level - 1); i++) {

        // Isoliert das Bit an Position 'i' durch Rechts-Shift und bitweise UND-Verknüpfung
        uint8_t currentBit = (sequence >> i) & 0x01;

        GPIOB->ODR |= (1 << 8); // LED aktivieren

        // Zeitliche Unterscheidung der Signalarten
        if (currentBit == 1) {
            timerDelayMs(1000); // 1 = langes Signal
        } else {
            timerDelayMs(350);  // 0 = kurzes Signal
        }

        GPIOB->ODR &= ~(1 << 8); // LED deaktivieren
        timerDelayMs(1000);      // Pause zur sauberen optischen Trennung der Bits
    }
}
