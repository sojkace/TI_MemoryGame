#include "stm32g431xx.h"




// Eigene Module einbinden (sobald erstellt)
#include "ADC_random_numbers.h"
#include "led.h"
// #include "button.h"
#include "tools.h"

/* --- 1. ZUSTANDSDEFINITIONEN (FSM) --- */
typedef enum {
    STATE_INIT,
    STATE_PLAY_SEQUENCE,
    STATE_WAIT_FOR_INPUT,
    STATE_LEVEL_SUCCESS,
    STATE_GAME_OVER
} GameState_t;


/* --- GLOBALEN VARIABLEN --- */
volatile GameState_t currentState = STATE_INIT;
volatile uint8_t currentLevel = 1; // Start bei Level 1 (max 5)
volatile uint32_t randomSeed = 0;

/* --- HILFSFUNKTIONEN FÜR PERIPHERIE --- */
void System_Init(void) {
    // Hier kommt später die grundlegende Taktkonfiguration hin
    RCC->CR |= (1 << 24); // PLL aktivieren (wie in den Vorübungen)

}


int main(void) {
    // System-Grundkonfiguration beim Starten des Controllers
    System_Init();
    initHardwareDelay();
    initRandomNumber();
    initLEDs();

    /* * Hier werden später die Module initialisiert, z.B.:
     * initRandomNumber();
     * initLEDs();
     * initButtons();
     */

    /* --- HAUPTSCHLEIFE (ZUSTANDSMASCHINE) --- */
    while (1) {
        switch (currentState) {

            case STATE_INIT:
                // 1. Level auf 1 setzen
                currentLevel = 1;

                // 2. Zufalls-Seed über ADC holen
                randomSeed = generateRandomNumber();
                // 3. Erste Sequenz erzeugen

                // Wenn das Setup fertig ist, gehen wir direkt in die Ausgabe
                currentState = STATE_PLAY_SEQUENCE;
                break;

            case STATE_PLAY_SEQUENCE:
                // 1. EXTI (Taster-Interrupts) deaktivieren, damit der Spieler
                // während der Ausgabe nicht dazwischenfunken kann.

                // 2. Die generierte Bitmaske/Sequenz über den Timer abarbeiten
                // 3. LEDs entsprechend ein- und ausschalten
            	led_playSequence(randomSeed, currentLevel);

                // Sobald die Sequenz komplett ausgegeben wurde:
                currentState = STATE_WAIT_FOR_INPUT;
                break;

            case STATE_WAIT_FOR_INPUT:
                // 1. EXTI (Taster-Interrupts) aktivieren
                // 2. Warten auf die Eingabe des Spielers

                /*
                 * Die Überprüfung der Taster passiert primär in den Interrupt-Service-Routinen (ISR).
                 * Ein falscher Tastendruck setztcurrentState = STATE_GAME_OVER;
                 * Das letzte korrekte Bit setzt currentState = STATE_LEVEL_SUCCESS;
                 */
                break;

            case STATE_LEVEL_SUCCESS:
                // 1. LED 2x lang blinken lassen (Erfolgs-Feedback)
            	led_blinkSuccess();
                // 2. Level erhöhen (Maximal bis Level 5)
                if (currentLevel < 5) {
                    currentLevel++;
                }

                // 3. Kurze Pause (z.B. 3 Sekunden warten)
                timerDelayMs(1000);

                // Nächstes Level startet mit der neuen Sequenz
                currentState = STATE_PLAY_SEQUENCE;
                break;

            case STATE_GAME_OVER:
                // 1. LED 3x schnell blinken lassen (Fehler-Feedback)
            	led_blinkGameOver();
                // 2. Level zurücksetzen
                currentLevel = 1;

                // 3. Warten (z.B. 3 Sekunden) oder auf Reset-Taste reagieren
                timerDelayMs(1000);

                // Zurück zum Startzustand
                currentState = STATE_INIT;
                break;

            default:
                // Sicherheitsnetz, falls mal ein undefinierter Zustand erreicht wird
                currentState = STATE_INIT;
                break;
        }
    }
}
