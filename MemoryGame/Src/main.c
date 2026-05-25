#include "stm32g431xx.h"
#include "random.h"
#include "LED_blink.h"
#include "LED_blink_timer.h"
#include "LED_Blink_Interrupt.h"

int main(void) {
    // Hier würde deine Hardware-Initialisierung (init) stehen

//    uint32_t meineZufallszahl1 = 0;
//    uint32_t meineZufallszahl2 = 0;
//    uint32_t meineZufallszahl3 = 0;
//    uint32_t meineZufallszahl4 = 0;

    while (1) {
        // Du rufst einfach nur noch den Namen auf. Herrlich übersichtlich!
//        meineZufallszahl1 = generateRandomNumber();
//        meineZufallszahl2 = generateRandomNumber();
//        meineZufallszahl3 = generateRandomNumber();
//        meineZufallszahl4 = generateRandomNumber();
        startLEDBlinkInterrupt();
        //startBlinken();

        // Jetzt kannst du mit der Zahl dein Memory-Level generieren
    }
}
