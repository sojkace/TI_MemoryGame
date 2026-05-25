/*

 LED_Blink_Interrupt.c

 Created on: 25.05.2026

 Author: sojka
 */

#include "stm32g431xx.h" // Zentrale Header-Datei für den STM32G431

void initLEDBlinkInterrupt(void) {
	// --- 1. GPIO KONFIGURATION ---
	RCC->CR |= 1 << 24;      // PLL (Systemtakt) aktivieren
	RCC->AHB2ENR |= 1 << 1;  // Takt für Port B (AHB2 Bus) einschalten
	GPIOB->MODER = 0x00010000; // PB8 als Ausgang konfigurieren

// --- 2. TIMER 3 GRUNDKONFIGURATION ---
	RCC->APB1ENR1 |= 1 << 1; // Takt für Timer 3 aktivieren
	TIM3->PSC = 99;        // Vorteiler (Teilt den Takt durch 100)
	TIM3->ARR = 60000;     // Auto-Reload: Zählt bis 60000, dann Überlauf
	TIM3->CR1 = 1 << 0;      // Timer 3 physikalisch starten (CEN-Bit setzen)

// --- 3. INTERRUPT KONFIGURATION ---
	// DIER = DMA/Interrupt Enable Register.
	// Bit 0 ist das UIE-Bit (Update Interrupt Enable). Es erlaubt dem Timer 3,
	// bei jedem Überlauf (ARR erreicht) ein Hardware-Interrupt-Signal zu feuern.
	TIM3->DIER = 1 << 0;

	// FEHLER / STOLPERFALLE 1:
	// Du hast oben Timer 3 konfiguriert, aktivierst hier im NVIC
	// (Nested Vectored Interrupt Controller) aber den Interrupt für Timer 2!
	// Richtig muss es lauten: NVIC_EnableIRQ(TIM3_IRQn);
	// Ohne diese Korrektur ignoriert die CPU den Timer 3 komplett.
	NVIC_EnableIRQ(TIM2_IRQn);

	// Globale Freigabe: Erlaubt der CPU generell, auf Interrupts zu reagieren.
	__enable_irq();

}

void startLEDBlinkInterrupt(void) {
	initLEDBlinkInterrupt(); // Ruft die Initialisierung auf

	// Die Main-Loop ist jetzt komplett leer!
	// Das ist der enorme Vorteil von Interrupts: Die CPU könnte hier jetzt
	// in den Sleep-Modus gehen (Strom sparen) oder andere Berechnungen
	// durchführen. Das Blinken passiert völlig entkoppelt im Hintergrund.
	while (1) {
	}

}

// ISR = Interrupt Service Routine für Timer 3.
// Diese Funktion ruft die Hardware automatisch auf, sobald der Timer überläuft.
void TIM3_IRQHandler(void) {

	// FEHLER / STOLPERFALLE 2:
	// SR = Status Register. Bit 0 ist das UIF (Update Interrupt Flag).
	// Es muss zwingend manuell gelöscht werden, sonst hängt die CPU für immer
	// in diesem Interrupt fest.
	// ABER: '0<<0' ist mathematisch 0. Der Code führt also aus: TIM3->SR &= 0;
	// Das löscht ungewollt ALLE Flags im Status Register auf einmal.
	// Sauberer und sicherer: TIM3->SR &= ~(1<<0);
	TIM3->SR &= 0 << 0;

	// ODR = Output Data Register.
	// Das Zirkumflex (^) steht für ein bitweises XOR (Exklusiv-ODER).
	// Ein XOR mit 1 kippt den Zustand des Bits (aus 1 wird 0, aus 0 wird 1).
	// Das ist der eleganteste Befehl, um eine LED blinken ("toggeln") zu lassen,
	// da du dir nicht mehr merken musst, ob sie gerade an oder aus war!
	GPIOB->ODR ^= 1 << 8;

}
