// Davey Taylor, Arduino Verkstad AB
// Power Management Arduino Objects

#include "CPU.h"
#include <stm32f2xx_exti.h>
#include <misc.h>


Arduino_CPU CPU;

void
Arduino_CPU::setWakeSource(WakeSource src, bool enable) {
  static bool enabled[NUM_WAKE_SOURCES];
  
  if (enable != enabled[src]) {
    enabled[src] = enable;
    uint8_t port = 0;
    uint8_t pin = 0;
    switch (src) {
      case POWER_BUTTON:
        // PB11
        port = 1;
        pin = 11;
        break;
      case ACCELEROMETER:
        // PB9
        port = 1;
        pin = 9;
        break;
      case TOUCH_SENSOR:
        // PC6
        port = 2;
        pin = 6;
        break;
      case BLUETOOTH:
        // PC13
        port = 2;
        pin = 13;
        break;
      case USB_POWER:
        // PA9
        port = 0;
        pin = 9;
        break;
      case WAKE_PIN:
        // PA0
        port = 0;
        pin = 0;
        break;
      default:
        return;
    }
    if (enable) {
      wake_source_set(port, pin);
    } else {
      wake_source_clear(pin);
    }
  }
}

void
Arduino_CPU::setLowPowerMode(LowPowerMode mode) {
  switch (mode) {
    case MODE_WATCH:
      // Disable interrupts generating events to simplify
      // sleep/stop wake logic
      NVIC_SystemLPConfig(NVIC_LP_SEVONPEND, DISABLE);
  
      // Disable standby to ensure that we only go into stop mode
      NVIC_SystemLPConfig(NVIC_LP_SLEEPDEEP, DISABLE);
  
      // Disable sleep-on-exit behavior, for now
      NVIC_SystemLPConfig(NVIC_LP_SLEEPONEXIT, DISABLE);

      // Disable flash when entering stop mode.
      // Saves power in exchanged for increased wake latency
      PWR_FlashPowerDownCmd(ENABLE);
  
      // Disable the backup SRAM regulator to save power
      PWR_BackupRegulatorCmd(DISABLE);
      break;
    default:
      break;
  }
}

void
Arduino_CPU::clearEvents() {
  cli();
  EXTI_ClearFlag(0xFFFFFFFF);
  __SEV();
  __WFE();
};

void
Arduino_CPU::waitForEvent() {
  PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);

  EXTI_ClearFlag(0xFFFFFFFF);
  cli();
};

void
Arduino_CPU::reboot() {
  ((void(*)())(((uint32_t*)SCB->VTOR)[1]))();
}