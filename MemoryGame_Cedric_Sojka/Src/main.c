#include "stm32g431xx.h"




// Eigene Module einbinden (sobald erstellt)
#include "ADC_random_numbers.h"
#include "led.h"
#include "button.h"
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
volatile uint8_t inputBitIndex = 0; // 2. ANPASSUNG: Zählt, das wievielte Bit der Spieler gerade eingibt

/* --- HILFSFUNKTIONEN FÜR PERIPHERIE --- */
void System_Init(void) {
    // Hier kommt später die grundlegende Taktkonfiguration hin
    RCC->CR |= (1 << 24); // PLL aktivieren (wie in den Vorübungen)

}

/* --- 3. ANPASSUNG: LOGIK-FUNKTION FÜR DIE INTERRUPTS ---
 * Diese Funktion wird automatisch aus der button.c aufgerufen,
 * sobald eine steigende Flanke an PA0 oder PA2 erkannt wird.
 */
void processPlayerInput(uint8_t bitEntered) {
    // Eingaben komplett ignorieren, wenn das Spiel nicht im Warte-Modus ist
    if (currentState != STATE_WAIT_FOR_INPUT) {
        return;
    }


    // Das erwartete Bit aus dem randomSeed herausschneiden (Index wandert mit)
    uint8_t expectedBit = (randomSeed >> inputBitIndex) & 0x01;

    if (bitEntered == expectedBit) {
        // Bit war korrekt!

        inputBitIndex++;

        // Prüfen, ob das Level vollständig erfolgreich eingegeben wurde
        if (inputBitIndex >= (currentLevel + 4)) {
            currentState = STATE_LEVEL_SUCCESS;
        } else {
        	led_inputBlink();
        }
    } else {
        // Bit war falsch -> Sofortiger Wechsel zu Game Over
        currentState = STATE_GAME_OVER;
    }
}


int main(void) {
    // System-Grundkonfiguration beim Starten des Controllers
    System_Init();
    initHardwareDelay();
    initRandomNumber();
    initLEDs();
    initButtons();

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

            	// 5. ANPASSUNG: Den Eingabe-Zähler für das neue Level auf 0 zurücksetzen
				inputBitIndex = 0;

                // Sobald die Sequenz komplett ausgegeben wurde:
                currentState = STATE_WAIT_FOR_INPUT;
                break;

            case STATE_WAIT_FOR_INPUT:
            	/* In diesem Zustand macht die Hauptschleife absolut gar nichts.
				 * Der Controller wartet passiv. Die Zustandsänderung passiert
				 * ausschließlich im Hintergrund über die processPlayerInput-Funktion.
				 */
                break;

            case STATE_LEVEL_SUCCESS:
                // 1. LED 2x lang blinken lassen (Erfolgs-Feedback)
            	led_blinkSuccess();
                // 2. Level erhöhen (Maximal bis Level 5)
                if (currentLevel < 5) {
                    currentLevel++;
                } else {
                	currentState = STATE_INIT;
                }

                // 3. Kurze Pause (z.B. 3 Sekunden warten)
                timerDelayMs(3000);

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
