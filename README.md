# ButterKnife Test Bed

A test bed interface for the Raspberry Pi Pico with oscilloscope, settings, and button test functionality.

## Recent Improvements

### Button Test Mode Fix
- Fixed issue where button test mode would return to main menu when testing buttons
- Added check to skip button handling in button test mode:
```cpp
// Skip button check if in button test mode
if (currentState == BUTTON_TEST_MODE) {
    return;
}
```
- Added clear exit instruction in button test mode:
```cpp
display.setCursor(0, 56);
display.print(F("Press encoder to exit"));
```

### Encoder and Button Handling
- Improved encoder responsiveness by reducing rate limiting from 10ms to 5ms
- Added proper debouncing for all buttons
- Fixed state management to prevent unwanted returns to main menu

### Pin Assignments
Current pin assignments for the Raspberry Pi Pico:
```cpp
#define ENCODER_A_PIN 2    // GP2 on the Pico
#define ENCODER_B_PIN 3    // GP3 on the Pico
#define ENCODER_BUTTON_PIN 4  // GP4 on the Pico
#define BUTTON1_PIN 5      // GP5 on the Pico
#define BUTTON2_PIN 6      // GP6 on the Pico
#define BUTTON3_PIN 7      // GP7 on the Pico
#define ANALOG_IN 26       // ADC0
#define ANALOG_IN2 27      // ADC1
```

### Key Features
- Oscilloscope mode with dual channel support
- Settings mode for scope configuration
- Button test mode for hardware testing
- Encoder-based menu navigation
- OLED display interface

### Recent Bug Fixes
1. Fixed oscilloscope running when not in scope mode
2. Fixed button test mode interference
3. Improved encoder responsiveness
4. Fixed menu navigation issues
5. Added proper state management

### Notes
- The encoder can use any digital I/O pins on the RP2040
- All GPIO pins support interrupts for the encoder
- Internal pull-ups are used for all buttons and encoder
- I2C pins (GP0-GP1) are used for the display 