#include "stm32g431xx.h"
#include "ADC_random_numbers.h"
#include "tools.h"

/* --- INITIALISIERUNG DES ADC FÜR DIE ZUFALLSGENERIERUNG --- */
void initRandomNumber(void) {
    // Takt für den Peripherie-Bus B (LEDs) und A (ADC-Eingang) aktivieren
    RCC->AHB2ENR |= (1 << 1) | (1 << 0);

    // PB8 als digitalen Ausgang konfigurieren (wird vom ADC-Modul zur Diagnose/Kontrolle mitinitialisiert)
    GPIOB->MODER &= ~(3 << 16);
    GPIOB->MODER |= (1 << 16);

    // PA3 (verbunden mit ADC1_IN4) in den analogen Modus versetzen
    // Ein offener analoger Pin fängt elektromagnetisches Rauschen auf ("White Noise")
    GPIOA->MODER |= (3 << 6);

    // Taktsignale für die ADC-Module aktivieren
    RCC->AHB2ENR |= (1 << 13);

    // ADC-Takt auf hclk/2 konfigurieren (sicherstellen, dass der ADC im spezifizierten Bereich arbeitet)
    ADC12_COMMON->CCR |= (1 << 17);

    // ADC aus dem Stromsparmodus (Deep Power Down) aufwecken
    ADC1->CR &= ~(1 << 29);

    // ADC-Sequenzer konfigurieren: Die erste (und einzige) Wandlung erfolgt an Kanal 4 (PA3)
    ADC1->SQR1 = (4 << 6); // Korrigiert von 0x0100 auf die saubere Bitmaske für Kanal 4

    // Internen Spannungsregler des ADC aktivieren und Stabilisierungszeit abwarten
    ADC1->CR |= (1 << 28);
    timerDelayMs(10);

    // ADC-Modul scharfschalten
    ADC1->CR |= (1 << 0);
}

/* --- GENERIERUNG EINER ECHTEN 32-BIT-ZUFALLSZAHL --- */
uint32_t generateRandomNumber(void) {
    uint32_t rand = 0;
    uint32_t currentBit = 0;

    // Erneute Stabilisierungszeit garantieren, bevor Messungen vorgenommen werden
    timerDelayMs(10);

    /* * Aufbau einer 32-Bit-Zufallszahl:
     * Da das physikalische Rauschen auf dem analogen Pin am stärksten im
     * niederwertigsten Bit (LSB) der 12-Bit-Wandlung auftritt, wird dieses
     * LSB isoliert. In 32 iterativen Messungen wird so Bit für Bit eine
     * echte, hardwarebasierte Zufallszahl (True Random Number) generiert.
     */
    for (int i = 0; i < 32; i++) {
        // Wandlung an ADC1 manuell anstoßen
        ADC1->CR |= (1 << 2);

        // Warten (Polling), bis das End-Of-Conversion (EOC) Flag gesetzt ist
        while (!(ADC1->ISR & (1 << 2))) {
            // ...
        }

        // Das unterste Bit (LSB) der 12-Bit-Wandlung maskieren und extrahieren
        currentBit = ADC1->DR & 0x01;

        // Das extrahierte Zufallsbit an die entsprechende Position (i) schieben
        currentBit = currentBit << i;

        // Das Bit in die finale 32-Bit-Zufallszahl einfügen
        rand |= currentBit;
    }

    return rand;
}
