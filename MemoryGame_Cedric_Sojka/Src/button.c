#include "stm32g431xx.h"
#include "button.h"

/* --- GLOBALE VARIABLEN (ZUSTAND DER ENTPRELLUNG) --- */
volatile uint32_t lastTouchTime_PA0 = 0;
volatile uint32_t lastTouchTime_PA2 = 0;

// Verknüpfung zur zentralen Spiellogik in main.c
extern void processPlayerInput(uint8_t bitEntered);

/* --- INITIALISIERUNG DER TOUCH-PINS --- */
void initButtons(void) {
    // Takt für GPIO Port A aktivieren
    RCC->AHB2ENR |= (1 << 0);

    // PA0 und PA2 als digitale Eingänge konfigurieren (Bitmaske: 00)
    GPIOA->MODER &= ~((3 << 0) | (3 << 4));

    // Interne Pull-Down-Widerstände aktivieren (Bitmaske: 10).
    // Verhindert undefinierte Spannungspegel (Floating) an den Pins,
    // da diese als reine kapazitive Berührungssensoren genutzt werden.
    GPIOA->PUPDR &= ~((3 << 0) | (3 << 4));
    GPIOA->PUPDR |=  ((2 << 0) | (2 << 4));

    // Takt für das System-Konfigurationsmodul (SYSCFG) aktivieren
    RCC->APB2ENR |= (1 << 0);

    // EXTI-Multiplexer konfigurieren: PA0 auf EXTI0 und PA2 auf EXTI2 routen
    SYSCFG->EXTICR[0] &= ~((0xF << 0) | (0xF << 8));

    // Interrupts für EXTI-Linie 0 und 2 demaskieren (freischalten)
    EXTI->IMR1 |= (1 << 0) | (1 << 2);

    // Trigger auf steigende Flanke konfigurieren (Auslösung bei Berührung / 3.3V)
    EXTI->RTSR1 |= (1 << 0) | (1 << 2);

    // EXTI-Interrupts im globalen NVIC freischalten
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);
}

/* --- INTERRUPT SERVICE ROUTINEN (ISR) --- */

// ISR für PA0: Eingabe der logischen "1" (langes Blinken)
void EXTI0_IRQHandler(void) {
    // Verifizieren, ob EXTI0 der tatsächliche Auslöser war
    if (EXTI->PR1 & (1 << 0)) {
        // Hardware-Interrupt-Flag sofort löschen (notwendig vor Beenden der ISR)
        EXTI->PR1 = (1 << 0);

        // Software-Entprellung & Blockierzeit (1000 ms).
        // Unterdrückt Mehrfachauslösungen durch unsaubere Berührungen
        // und überbrückt die asynchrone Wartezeit des optischen Feedbacks.
        if ((TIM2->CNT - lastTouchTime_PA0) > 1000) {

            processPlayerInput(1);

            // Zeitstempel aktualisieren, NACHDEM die Logik verarbeitet wurde
            lastTouchTime_PA0 = TIM2->CNT;

            // Eventuelle Stör-Flanken, die während der Verarbeitung (LED-Feedback)
            // registriert wurden, als Hardware-Post-it restlos löschen.
            EXTI->PR1 = (1 << 0);
        }
    }
}

// ISR für PA2: Eingabe der logischen "0" (kurzes Blinken)
void EXTI2_IRQHandler(void) {
    // Verifizieren, ob EXTI2 der tatsächliche Auslöser war
    if (EXTI->PR1 & (1 << 2)) {
        EXTI->PR1 = (1 << 2);

        // Entprell- und Blockierfenster evaluieren
        if ((TIM2->CNT - lastTouchTime_PA2) > 1000) {

            processPlayerInput(0);

            // Störungsfreie Reaktivierung des nächsten Eingabefensters
            lastTouchTime_PA2 = TIM2->CNT;
            EXTI->PR1 = (1 << 2);
        }
    }
}
