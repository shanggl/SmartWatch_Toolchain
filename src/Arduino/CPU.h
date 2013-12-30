// Davey Taylor, Arduino Verkstad AB
// Power Management Arduino Objects

#ifndef _CPU_H_
#define _CPU_H_
#include "../Arduino.h"
#include "../driver_power.h"
#include "../driver_adc.h"

enum WakeSource {
  POWER_BUTTON = 0,
  ACCELEROMETER,
  TOUCH_SENSOR,
  BLUETOOTH,
  USB_POWER,
  WAKE_PIN,
  NUM_WAKE_SOURCES
};

enum LowPowerMode {
  MODE_WATCH
};

class Arduino_CPU {
  public:
    // Change CPU speed
    inline void setSpeed(const ClockDef_t *clk) {
      cpu_reclock(clk);
    };
    
    void setWakeSource(WakeSource src, bool enable);
    inline void enableWakeSource(WakeSource src) {
      setWakeSource(src, true);
    };
    inline void disableWakeSource(WakeSource src) {
      setWakeSource(src, false);
    };
    
    void setLowPowerMode(LowPowerMode mode);
    
    void clearEvents();
    void waitForEvent();
    void reboot();
};

extern Arduino_CPU CPU;

#endif