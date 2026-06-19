#include "stm32g431xx.h"
#include <main.h>

volatile uint8_t lv=1;
volatile uint16_t tk=0;
volatile uint8_t phase=0;
volatile uint32_t rnd=0;
volatile uint8_t rnd_rdy=0;
volatile uint8_t cur_bit=0;
volatile uint8_t bit_rdy=0;
volatile uint16_t pos=0;
volatile uint8_t seq_end=0;
volatile uint8_t inp_bit=0;
volatile uint8_t inp_flag=0;
volatile uint8_t debounce=0;
volatile uint8_t pause3=0;
volatile uint8_t sec_cnt=0;
volatile uint8_t flash3=0;
volatile uint8_t flash_cnt=0;

static void delay_ms(int n){
  SysTick->LOAD=4000;
  SysTick->VAL=0;
  SysTick->CTRL=5;
  for(int i=0;i<n;i++)while(!(SysTick->CTRL&0x10000));
  SysTick->CTRL=0;
}

static void rng_gen(uint32_t len){
  uint32_t v=0;
  for(uint32_t i=0;i<len;i++){
    ADC1->CR|=1<<2;
    while(!(ADC1->ISR&4));
    v|=(ADC1->DR&1)<<i;
  }
  rnd=v;
  rnd_rdy=1;
}

static void next_bit(void){
  if(pos<lv+4){
    cur_bit=(rnd>>pos)&1;
    pos++;
    bit_rdy=1;
    if(phase&&pos==lv+4){seq_end=1;pos=0;}
  }else{seq_end=1;pos=0;bit_rdy=0;}
}

#define LED_ON()  (GPIOB->ODR=1<<8)
#define LED_OFF() do{GPIOB->ODR=0;bit_rdy=0;}while(0)

static void irq_on(void){EXTI->IMR1|=1<<0;EXTI->IMR1|=1<<8;EXTI->PR1|=1<<0;EXTI->PR1|=1<<8;}
static void irq_off(void){EXTI->IMR1&=~(1<<0);EXTI->IMR1&=~(1<<8);}

static void lv_up(void){if(lv<5)lv++;}

static void reset_all(void){lv=1;tk=0;phase=0;rnd=0;rnd_rdy=0;cur_bit=0;bit_rdy=0;pos=0;seq_end=0;inp_bit=0;inp_flag=0;debounce=0;}

static void adc_init(void){
  RCC->CR|=1<<24;
  RCC->AHB2ENR|=1<<1;
  GPIOB->MODER=0x00010000;
  RCC->AHB2ENR|=1<<0;
  GPIOA->MODER|=0xC0;
  RCC->AHB2ENR|=1<<13;
  ADC12_COMMON->CCR|=1<<17;
  ADC1->CR&=~(1<<29);
  ADC1->SQR1=0x0100;
  ADC1->CR|=1<<28;
  delay_ms(1);
  ADC1->CR|=1<<0;
}

static void tim_init(void){
  RCC->APB1ENR1|=1<<1;
  TIM3->PSC=1599;
  TIM3->ARR=499;
  TIM3->CR1=1;
  TIM3->DIER=1;
  NVIC_EnableIRQ(TIM3_IRQn);
}

static void io_init(void){
  RCC->AHB2ENR|=1<<0;
  GPIOA->MODER&=~0x3;
  GPIOA->MODER&=~0x30000;
  RCC->APB2ENR|=1<<0;
  GPIOA->PUPDR&=~1;
  GPIOA->PUPDR&=~(1<<8);
  SYSCFG->EXTICR[0]&=~0xF;
  SYSCFG->EXTICR[3]&=~0xF;
  EXTI->FTSR1|=1<<0;
  EXTI->FTSR1|=1<<8;
  irq_off();
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI9_5_IRQn);
}

static void hw_init(void){adc_init();tim_init();io_init();__enable_irq();}

void main(void){hw_init();while(1);}

void TIM3_IRQHandler(void){
  TIM3->SR&=~1;
  if(!pause3&&!flash3){
    if(!phase){
      if(!tk){
        if(!rnd_rdy)rng_gen(lv+4);
        if(!bit_rdy&&!seq_end)next_bit();
        if(bit_rdy)LED_ON();
      }
      if(tk==6&&bit_rdy&&!cur_bit)LED_OFF();
      if(tk==19&&bit_rdy&&cur_bit)LED_OFF();
      if(tk==29&&seq_end){irq_on();seq_end=0;phase=1;}
    }else{
      if(inp_flag&&!debounce){tk=0;debounce=1;next_bit();if(inp_bit==cur_bit)LED_ON();else{irq_off();reset_all();flash3=1;pause3=1;}}
      if(tk==20&&debounce){debounce=0;inp_flag=0;tk=0;}
      if(tk==6&&bit_rdy&&!cur_bit)LED_OFF();
      if(tk==19&&bit_rdy&&cur_bit)LED_OFF();
      if(tk==20&&seq_end){irq_off();seq_end=0;phase=0;bit_rdy=0;rnd_rdy=0;lv_up();pause3=1;}
    }
  }
  if(flash3){
    if(!tk)LED_ON();
    if(tk==19){LED_OFF();flash_cnt++;if(flash_cnt==3){flash3=0;flash_cnt=0;}}
  }
  if(pause3&&!flash3){
    if(tk==19||tk==39){sec_cnt++;if(sec_cnt==3){pause3=0;sec_cnt=0;}}
  }
  tk=(tk+1)%40;
}

void EXTI0_IRQHandler(void){EXTI->PR1|=1<<0;inp_bit=1;inp_flag=1;}
void EXTI9_5_IRQHandler(void){if(EXTI->PR1&(1<<8)){EXTI->PR1|=1<<8;inp_bit=0;inp_flag=1;}}
