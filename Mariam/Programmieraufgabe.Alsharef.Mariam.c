/*Teile dieses Codes wurden von den bereitgestellten Programmen der Vorlesung übernommen.
*/
#include "stm32g431xx.h"

int level = 1;
int ranNum[9]; 
int inputIndex = 0;
int userInput = -1;
int gameMode = 0;
int counter = 0;
int currentBit = 0;
int ledOn = 0;
int blinkState = 0;
  
void systickDelayMs(int n)
{
 SysTick->LOAD = (SystemCoreClock / 1000) - 1; // Load clockcycless per millisecond
  SysTick->VAL = 0;             // Clear current counting register
  SysTick->CTRL = 0x5;          // Enable Systick
  
    
  for(int i =0;i<n;i++)
    {                           // Wait for 1ms until the COUNT flag is set
      while((SysTick->CTRL & 0x10000) ==0){} 
    }
  SysTick->CTRL =0;             // Disable Systick
}


void PortInit(){
  RCC->CR |= 1<<24;             // Bit24: Enable RCC clock
  RCC->AHB2ENR |= 1<<1; 	// Bit1: Enable clocks for GPIOB
 GPIOB->MODER = 0x00010000; 	// Set as digital output
 

  // Port A Initializiation: Input Port-Pins
  RCC->AHB2ENR |= 1<<0; 	// Enable clocks for GPIOA
  GPIOA->MODER &= ~ (3 << 0); 
  GPIOA->MODER &= ~(3 << 4);     // PA0 und PA2 als Input konfigurieren

  RCC->APB2ENR |= 1<<0; 	// Enable SYSCFG clock
  
  GPIOA->PUPDR &= ~(1<<0);      // Disable PA0 pull-up/pull-down
  GPIOA->PUPDR &= ~(1<<2);      // Disable PA2 pull-up/pull-down
  
  SYSCFG->EXTICR[1] &= ~(0xF);  // Select Port A as interrupt source for EXTI line 0 
  SYSCFG->EXTICR[3] &= ~(0xF);  // Select Port A as interrupt source for EXTI line 2 

  EXTI->IMR1 |= 1<<0;           // Enable EXTI line 0
  EXTI->IMR1 |= 1<<2;           // Enable EXTI line 2

  EXTI->RTSR1 |= 1<<0;          // Enable rising edge for trigger
  EXTI->RTSR1 |= 1<<2;          // Enable rising edge for trigger
}

void TimerInit(){
    // Timer3 Initializiation
  RCC->APB1ENR1 |= 1<<1;        // Bit1: Enable clocks for TIM3
  TIM3->PSC = (SystemCoreClock/1000) -1;              // Internal clock divided by 1000 
                                
  TIM3->ARR = 9;            // Counts from 0 to 9 -> ~1ms
  TIM3->CR1 = 1<<0;             // Enable TIM3 counter
}

void ADCinit(){ 
  
  // ADC Initializiation: Conversion
  RCC->AHB2ENR |= 1<<13; 	// Bit13: Enable clocks for ADC1 & ADC2
  ADC12_COMMON->CCR |= 1<<17;   // Bit17: hclk/2 clock mode for ADC1 & ADC2
  
  ADC1->CR &= ~(1<<29);         // Disable ADC1 deep power down mode
  ADC1->SQR1 = 0x0100;          // First conversion ADC1_IN4 == GPIOA.3
  ADC1->CR |= 1<<28;            // Enable voltage regulator 
  systickDelayMs(1);            // Wait 1 ms to start-up voltage regulator
  
  ADC1->CR |= 1<<0;             // Enable ADC1
}

void EnableInterrupts(){
  //PortInterrupts 
  EXTI->IMR1 |= 1<<0;           // Enable EXTI line 0
  EXTI->IMR1 |= 1<<2;           // Enable EXTI line 2
  
  EXTI->RTSR1 |= 1<<0;          // Enable rising edge for trigger
  EXTI->RTSR1 |= 1<<2;          // Enable rising edge for trigger
  // Enable Interrupts
  TIM3->DIER = 1<<0;            // TIM3 update interrupt enabled
  NVIC_EnableIRQ(TIM3_IRQn);    // Enable TIM3 interrupt vector in NVIC
  NVIC_EnableIRQ(EXTI0_IRQn) ;
  NVIC_EnableIRQ(EXTI2_IRQn);
  __enable_irq();               // Enable *all* interrupts (global)
}                               // NVIC (= nested vector interrupt controller)


void init(){
  
PortInit();
ADCinit();
TimerInit();
EnableInterrupts();
}


// LEDs on/off

void LedOn()
{
   GPIOB->ODR |= 1<<8;         // set GPIOB.8
   ledOn = 1;
}

void LedOff()
{
  GPIOB->ODR &= 0<<8;           // clear GPIOB.8
   ledOn = 0;     
} 


/**
Generates a random number and saves it as bits into the ranNum Array
*/
void randomNumber()
{
  unsigned int i = 0;
  

  systickDelayMs(10);                   // Wait 10 ms to stabilize ADC
  
    for (i = 0; i<(level+4); i++)
    {
       ADC1->CR |= 1 << 2;                 // Start analog-digital conversion
        while (!(ADC1->ISR & 0x04)) {}      // Wait for end of conversion
        ranNum[i] = ADC1->DR & 0x01;        // Store only the LSB (0 or 1)
        systickDelayMs(1);
    }
     
    } 




void main(void) 
{
  init();
 
 gameMode =0;
  while (1)      //endless loop, CPU-core does nothing
  {             // Game Logic is handled by interruptsx      
    }
  }



void TIM3_IRQHandler() {
  if (TIM3->SR & 1) {
    TIM3->SR &= ~1;
    counter++;
     if(counter > 350) counter = 350; //caps counter value
   

    switch (gameMode) {
    case 0: // start
      randomNumber();
      currentBit = 0;
      counter = 0;
      ledOn = 0;
      gameMode = 1;
      break;

    case 1: // show sequence
      if (currentBit >= level + 4) {
        gameMode = 2;
        inputIndex = 0;
        counter = 0;
    
        break;
      }

      if (ledOn == 0 && counter >= 20) {
        LedOn();
        counter = 0;
      }

      // 1 = long, 0 = short
      if (ledOn == 1) {
        if ((ranNum[currentBit] == 0 && counter >= 35) ||
            (ranNum[currentBit] == 1 && counter >= 100)) {
          LedOff();
          counter = 0;
          
          currentBit++;
        }
      }
      break;

case 2: // user input - check input
  if (userInput == 1 || userInput == 0) {
    
    if (ranNum[inputIndex] != userInput) {
      counter = 0;
      gameMode = 4;  // game over
     
    } else {
      LedOn();
      counter = 0; 
      inputIndex++;
    }
    
    userInput = -1;
  }

  if (ledOn == 1) {
    if ((counter >= 35 && ranNum[inputIndex -1] == 0) ||
        (counter >= 100 &&  ranNum[inputIndex-1] == 1)) {
      LedOff();
       
      counter = 0;

      if (inputIndex >= level + 4) {
        gameMode = 3;
      }
    }
  }
  break;


case 3: // level up
   if (counter >= 300) { //3sec break
     if (level < 5) {level++;
    gameMode = 0;
    counter = 0;
    LedOff();}
     else{
     LedOn();
  }
   }
  
  break;

    case 4: // game over 3x blink & 3 sec break
      if (blinkState <= 3){
      if (counter >= 30) {
      LedOn();
      if (counter >= 60) {
        LedOff();
  counter = 0;
  
   blinkState++;
      }
  
}
      }
 else {
 
  LedOff();
   if (counter >=300){ 
  level = 1;
  inputIndex = 0;
  gameMode = 0;
  counter = 0;
  blinkState = 0;
  LedOff();
   }
}
  break;

    }
  }
}



void EXTI2_IRQHandler(void)  // PA2: short (0)
{
  if (EXTI->PR1 & (1<<2)) {  
    EXTI->PR1 |= (1<<2);          // Clear interrupt
     systickDelayMs(50);           // Debouncing delay
    if (gameMode == 2 && ledOn == 0) {
      userInput = 1;
  }}
  }



void EXTI0_IRQHandler(void)  // PA0: long (1)
{
  if (EXTI->PR1 & (1<<0)) 
        {
          EXTI->PR1 |= (1<<0);    // Clear PR to re-enable EXTI interrupt
           systickDelayMs(50);           // Debouncing delay
           if (gameMode == 2 && ledOn == 0) {
      userInput = 0;
    }
        }
}
