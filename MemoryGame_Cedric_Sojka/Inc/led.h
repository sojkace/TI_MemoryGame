/*
 * led.h
 *
 *  Created on: 19.06.2026
 *      Author: sojka
 */

#ifndef LED_H_
#define LED_H_

void initLEDs(void);

void led_playSequence(uint32_t sequence, uint8_t level);
void led_blinkSuccess(void);
void led_blinkGameOver(void);
void led_inputBlink(void);

#endif /* LED_H_ */
