/*
 * random.h
 *
 *  Created on: 25.05.2026
 *      Author: sojka
 */

#ifndef RANDOM_H_
#define RANDOM_H_

// Einbinden der STM32-Bibliothek, damit die Register-Namen hier bekannt sind
#include "stm32g431xx.h"

// Das ist der Prototyp. Er sagt der main() nur, wie die Funktion heißt
// und was sie zurückgibt, aber noch nicht, wie sie funktioniert.
uint32_t generateRandomNumber(void);



#endif /* RANDOM_H_ */
