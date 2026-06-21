#include "stm32g431xx.h"

// Eigene Peripherie- und Logikmodule
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
	// Konfiguration des Systemtakts (Aktivierung der PLL)
    RCC->CR |= (1 << 24); // PLL aktivieren (wie in den Vorübungen)

}

/* --- SPIELLOGIK --- */
// Wird asynchron von den EXTI-Routinen aufgerufen, um Tastereingaben auszuwerten.
void processPlayerInput(uint8_t bitEntered) {
	// Taster-Interrupts während der Verarbeitung blockieren
	NVIC_DisableIRQ(EXTI0_IRQn);
	NVIC_DisableIRQ(EXTI2_IRQn);

	// Eingaben ignorieren, wenn das Spiel nicht auf eine Eingabe wartet
    if (currentState != STATE_WAIT_FOR_INPUT) {
        return;
    }

    // Optisches Feedback für die Berührung geben
    led_inputBlink(bitEntered);


    // Erwartetes Bit an der aktuellen Position der Sequenz extrahieren
    uint8_t expectedBit = (randomSeed >> inputBitIndex) & 0x01;

    // Abgleich der Eingabe mit der generierten Sequenz
    if (bitEntered == expectedBit) {
        inputBitIndex++;

        // Prüfen, ob die geforderte Anzahl an Bits für dieses Level erreicht wurde (Lvl 1 startet mit 5 Bits)
        if (inputBitIndex >= (currentLevel + 4)) {
            currentState = STATE_LEVEL_SUCCESS;
        } else {
        	// Level noch nicht abgeschlossen: Interrupts für die nächste Eingabe wieder freigeben
        	NVIC_EnableIRQ(EXTI0_IRQn);
			NVIC_EnableIRQ(EXTI2_IRQn);
        }
    } else {
    	// Fehlerhafte Eingabe führt zum sofortigen Spielende
        currentState = STATE_GAME_OVER;
    }
}

/* --- HAUPTPROGRAMM --- */
int main(void) {
	// Grundinitialisierung der Hardware-Ressourcen
    System_Init();
    initHardwareDelay();
    initRandomNumber();
    initLEDs();
    initButtons();

    /* --- ZUSTANDSMASCHINE --- */
    while (1) {
        switch (currentState) {

            case STATE_INIT:
                currentLevel = 1;
                randomSeed = generateRandomNumber();
                currentState = STATE_PLAY_SEQUENCE;
                break;

            case STATE_PLAY_SEQUENCE:
            	// Hardware-Taster deaktivieren, um Eingaben während der Ausgabe zu verhindern
            	NVIC_DisableIRQ(EXTI0_IRQn);
				NVIC_DisableIRQ(EXTI2_IRQn);

            	led_playSequence(randomSeed, currentLevel);

            	// Zähler für die bevorstehende Spielereingabe zurücksetzen
				inputBitIndex = 0;

                currentState = STATE_WAIT_FOR_INPUT;
                break;

            case STATE_WAIT_FOR_INPUT:
            	// Hardware-Flags löschen, um sofortiges Auslösen durch aufgestaute Interrupts zu vermeiden
				//EXTI->PR1 = (1 << 0) | (1 << 2);
				//NVIC_ClearPendingIRQ(EXTI0_IRQn);
				//NVIC_ClearPendingIRQ(EXTI2_IRQn);

				// Eingabebereitschaft wiederherstellen
            	NVIC_EnableIRQ(EXTI0_IRQn);
				NVIC_EnableIRQ(EXTI2_IRQn);
                break;

            case STATE_LEVEL_SUCCESS:
            	NVIC_DisableIRQ(EXTI0_IRQn);
				NVIC_DisableIRQ(EXTI2_IRQn);

            	led_blinkSuccess();

                if (currentLevel < 5) {
                    currentLevel++;
                }

                timerDelayMs(3000);
                currentState = STATE_PLAY_SEQUENCE;
                break;

            case STATE_GAME_OVER:
            	led_blinkGameOver();
                currentLevel = 1;

                timerDelayMs(1000);
                currentState = STATE_INIT;
                break;

            default:
            	// Sicherheitsrückfall bei undefiniertem Zustand
                currentState = STATE_INIT;
                break;
        }
    }
}
