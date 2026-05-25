/*

 LED_blink.c

 Created on: 25.05.2026

 Author: sojka
 */

#include "LED_blink.h" // Bindet die Header-Datei ein, in der vermutlich diese Funktionen (Prototypen) deklariert sind.

void initLEDBlink(void) {
	// RCC = Reset and Clock Control. Hierüber wird gesteuert, welche Bauteile des Chips Strom/Takt bekommen.
	RCC->CR |= 1 << 24; // Setzt Bit 24 im Control Register auf 1. Damit wird in der Regel der PLL (Phase-Locked Loop) aktiviert, der den Systemtakt generiert.
	RCC->AHB2ENR |= 1 << 1; // AHB2ENR = Advanced High-Performance Bus 2 Enable Register. Bit 1 aktiviert den Takt für GPIO Port B. Ohne diesen Befehl bleibt Port B "tot" und reagiert auf nichts.

	// MODER = Mode Register. Hier wird festgelegt, was der Pin tun soll (Eingang, Ausgang, Analog, etc.).
	// 0x00010000 (Hexadezimal) bedeutet, dass Bit 16 auf 1 gesetzt wird. Die Bits 16 und 17 sind für Pin 8 zuständig.
	// Die Kombination (Bit17=0, Bit16=1) schaltet Pin PB8 in den "General purpose output mode" (Ausgang).
	GPIOB->MODER = 0x00010000;

}

void delay(void) {
	long int i; // Deklariert eine Zählvariable. 'long int' garantiert, dass sehr große Zahlen gespeichert werden können.
	// Eine leere for-Schleife, die einfach nur von 0 bis 399.999 hochzählt.
	// Der Prozessor rechnet hier "sinnlos", was Zeit verbraucht und so als Pause fungiert.
	for (i = 0; i < 400000; i++)
		;
}

void startBlinken(void) {
	initLEDBlink(); // Ruft unsere Initialisierungs-Funktion von oben auf.

	// Endlosschleife. Die sogenannte "Main-Loop" oder "Super-Loop".
	while (1) {
		// ODR = Output Data Register. Hier wird der Strom am Pin an- oder ausgeschaltet.
		// Die Ver-Oderung (|=) mit 1 um 8 Stellen nach links verschoben (1<<8) setzt exakt Pin PB8 auf HIGH (1). Die LED geht an.
		GPIOB->ODR |= 1 << 8;

		delay(); // Wartet eine Weile (der Prozessor ist hier komplett blockiert).

		// ACHTUNG: '&= 0' bedeutet ein bitweises UND mit 0. Das setzt ALLE Bits auf 0.
		// Die LED an PB8 geht zwar aus, aber falls noch andere LEDs oder Motoren an Port B hängen,
		// werden diese ebenfalls gnadenlos ausgeschaltet!
		// Sauberer wäre: GPIOB->ODR &= ~(1<<8); (Löscht gezielt nur Bit 8).
		GPIOB->ODR &= 0;

		delay(); // Wartet wieder, während die LED aus ist.
	}

}
