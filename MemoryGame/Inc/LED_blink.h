/*
 * LED_blink.h
 *
 *  Created on: 25.05.2026
 *      Author: sojka
 */

#ifndef LED_BLINK_H_
#define LED_BLINK_H_

// Einbinden der STM32-Bibliothek, damit die Register-Namen hier bekannt sind
#include "stm32g431xx.h"

// Das ist der Prototyp. Er sagt der main() nur, wie die Funktion heißt
// und was sie zurückgibt, aber noch nicht, wie sie funktioniert.
void startBlinken(void);


#endif /* LED_BLINK_H_ */
