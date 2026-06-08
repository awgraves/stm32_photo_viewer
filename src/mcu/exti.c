#include "exti.h"
#include "registers.h"

// each callback slot matches the exti line num, which should match the pin num.
// for simplicity, just using 0-4 here, so 5 slots total.
#define CALLBACKS_LEN 5
static exti_callback_t callbacks[CALLBACKS_LEN];

void exti_configure(exti_config_t *config) {
  uint8_t pinno = PINNO(config->pin);

  // enable clock to SYSCFG
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFG;

  uint8_t exti_reg_idx = pinno >> 2; // divide by 4, each exti register holds 4
  uint8_t exti_reg_pos =
      pinno % 4; // find which group within the given exti reg

  // NOTE: always assuming port A for right now
  SYSCF->EXTICR[exti_reg_idx] |=
      SYSCFG_EXTI_INPUT_MASK(exti_reg_pos, SYSCFG_EXTI_INPUT_PA);

  // unmask the interrupt line
  EXTI->IMR |= BIT(pinno);

  // set triggers
  switch (config->trigger) {
  case EXTI_TRIGGER_EDGE_FALLING:
    EXTI->FTSR |= BIT(pinno);
    break;
  case EXTI_TRIGGER_EDGE_RISING:
    EXTI->RTSR |= BIT(pinno);
    break;
  default:
    break;
  }

  // enable the IRQ in NVIC
  NVIC->ISER |= NVIC_EXTI_BIT_FROM_PINNO(pinno);

  if (pinno < CALLBACKS_LEN) {
    // only supporting pins 0-4 right now
    callbacks[pinno] = config->callback;
  }
}

static inline void irq_handle(uint8_t num) {
  EXTI->PR = BIT(num);
  callbacks[num]();
}

/*
These provide hard definitions that override the weak ones
specified under mcu/startup.c
*/
void EXTI0_IRQHandler(void) { irq_handle(0); };
void EXTI1_IRQHandler(void) { irq_handle(1); }
void EXTI2_IRQHandler(void) { irq_handle(2); };
void EXTI3_IRQHandler(void) { irq_handle(3); }
void EXTI4_IRQHandler(void) { irq_handle(4); }
