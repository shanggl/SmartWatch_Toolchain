// Alternate firmware to display a watchface, with the option
// to go to sleep to save power while retaining the RTC time.

// Keep track of how long we've been idle (no touch events)
// to decide when to shutdown
uint32_t idle_ticks = 0;

void setup() {
  // Run standard startup procedure
  standardStartup();
  
  // Enable STOP mode with everything but the RTC disabled.
  CPU.setLowPowerMode(MODE_WATCH);
  
  // Allow the power button to wake us up from stop mode
  CPU.enableWakeSource(POWER_BUTTON);
  
  // Allow plugging into USB power to wake us up
  CPU.enableWakeSource(USB_POWER);
  
  // Init display
  OLED.begin();
  OLED.fillScreen(0x0000);
  OLED.setTextColor(0xFFFF, 0x0000);
  OLED.setTextSize(1);

  // Init stuff, show status
  Battery.begin();
  Battery.enableCharging();
  Touch.begin();
  Movement.begin();

  DateTime.begin();
  if(!DateTime.isRunning()) {
    // RTC has lost power, we need to set a new time
    DateTime.setDateTime(14, 1, 1, 1, 0, 0, 00);
  }
  
  idle_ticks = 0;
}

// Shutdown all of the application code and enter STOP mode,
// but keep the RTC running and reboot when the power button
// is pressed again
void softShutdown(bool notify=false) {
  // Turn off everything
  OLED.shutdown();
  Touch.shutdown();
  Movement.shutdown();
  if (notify) {
    // Buzz twice to indicate soft shutdown
    digitalWrite(BUZZER, HIGH);
    delay(125);
    digitalWrite(BUZZER, LOW);
    delay(125);
    digitalWrite(BUZZER, HIGH);
    delay(125);
    digitalWrite(BUZZER, LOW);
  }
  // Don't turn off until button is released
  while(digitalRead(BUTTON));

  CPU.clearEvents();
  
  // enter STOP mode until the power button is pressed
  CPU.waitForEvent();

  // Exited STOP mode - reboot
  CPU.reboot();
}

void loop() {
  char text[32];

  const uint8_t size = 3;
  const uint8_t height = 7;

  digitalWrite(&PIN_PB1, Battery.canCharge());
  
  // Clock
  OLED.setCursor(8, 64-height*size/2);
  OLED.setTextSize(size);
  DateTime.update();
  uint8_t hour = DateTime.hour();
  bool PM = false;
  if (hour > 12) {
    PM = true;
    hour -= 12;
  } else if (hour == 12) {
    PM = true;
  } else if (hour == 0) {
    hour = 12;
  }
  
  sprintf(text, "%2u:%02u", hour, DateTime.minute());
  OLED.println(text);
  
  OLED.setTextSize(1);
  OLED.setCursor(104, 64-height*size/2);
  sprintf(text, "%02u", DateTime.second());
  OLED.println(text);
  
  OLED.setCursor(104, 64+height*size/2-height);
  if (PM) {
    OLED.println("PM");
  } else {
    OLED.println("AM");
  }

  OLED.setCursor(88, 112);
  // Battery level as volts
  sprintf(text, "%.2fV", ((float)Battery.readMilliVolts()) / 1000.0);
  OLED.println(text);
 
  bool isTouched = false;
  if(Touch.read()) {
    if(Touch.isTouched()) {
      isTouched = true;
    }
  }
  
  // Keep track of how long since the last touch event
  if (isTouched) {
    idle_ticks = 0;
  } else {
    idle_ticks++;
  }
  
  // Don't bother trying to enter a soft-shutdown when USB
  // power is connected.
  if (digitalRead(&PIN_PA9)) {
    idle_ticks = 0;
  }
  
  // Sleep after ~30 seconds of inactivity
  if (idle_ticks >= 10 * 30) {
    softShutdown();
  }

  // Delay in low speed mode
  CPU.setSpeed(CPU_LS);
  delay(100);
  CPU.setSpeed(CPU_HS);

  // Shut down if button is pressed
  if(digitalRead(BUTTON)) {
    standardShutdown();
  }
}