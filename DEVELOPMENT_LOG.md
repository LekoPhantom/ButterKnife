# Development Log

## Troubleshooting and Improvements

### Initial Issue: Software Lockups
**User**: "I am noticing there are some times when the software locks up. I am wondering if you could help me troubleshoot it. I know it happens when the encoder is spun (possibly too fast)? I did notice a few delays. I was wondering if that is messing with the interrupt. Is the filter another possibility?"

**Changes Made**:
1. Removed unnecessary delays that could interfere with interrupts
2. Added rate limiting to prevent encoder overflow
3. Made the oscilloscope more efficient by only running when active

### Oscilloscope Optimization
**User**: "Could we also make sure the oscilloscope is not always running?"

**Changes Made**:
1. Added `oscilloscopeActive` flag
2. Only run oscilloscope when in OSCILLOSCOPE_MODE
3. Proper cleanup when exiting oscilloscope mode

### Button Test Mode Issues
**User**: "One issue is in the button test, sometimes this causes the app to go back to the main screen. So, I cannot see the buttons. This is not really consistent though."

**Changes Made**:
1. Added check to skip button handling in button test mode:
```cpp
// Skip button check if in button test mode
if (currentState == BUTTON_TEST_MODE) {
    return;
}
```
2. Added clear exit instruction in button test mode:
```cpp
display.setCursor(0, 56);
display.print(F("Press encoder to exit"));
```

### Pin Assignment Discussion
**User**: "Can the encoder use any DIO pins on the rpipico? Or is the interrupt specific? I want to edit the pin assignments so I can connect them with the header directly."

**Changes Made**:
Updated pin assignments to use more convenient GPIO pins:
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

### Prompt Quality Evaluation
**Discussion**: Evaluation of prompt quality on a scale of 1-4 (4 being proficient with distinction)

**Rating**: 3.5/4

**Strengths of the Prompts**:
1. Clear Problem Description
   - Specific conditions when issues occurred
   - Clear statement of the problem
   - Good technical context

2. Technical Context
   - Understanding of potential technical causes
   - Good questions about specific components
   - Appropriate follow-up observations

3. Specific Questions
   - Focused questions that helped narrow down solutions
   - Good technical specificity
   - Clear intent

4. Follow-up Observations
   - Good feedback on changes
   - Clear identification of new issues
   - Helpful context for further improvements

**Areas for Improvement**:
1. More Context
   - Could include hardware setup details
   - Could describe expected vs actual behavior
   - Could mention specific hardware models

2. Structured Problem Description
   - Could list symptoms in order of occurrence
   - Could mention previous fix attempts
   - Could include timing patterns

3. More Specific Technical Details
   - Could include error messages
   - Could specify timing of issues
   - Could mention specific patterns

**Example of an Improved Prompt**:
```
I'm working on a Pico-based oscilloscope with an OLED display and rotary encoder. I'm experiencing lockups with these specific symptoms:
1. Occurs when spinning the encoder quickly
2. No error messages, just freezes
3. Happens more frequently in oscilloscope mode

I've noticed:
- There are several delay() calls in the code
- The encoder uses interrupts
- The issue seems timing-related

Could this be related to interrupt conflicts with the delays? Or could it be a buffer overflow issue?
```

### Key Improvements Summary
1. Fixed oscilloscope to only run when needed
2. Made the encoder more responsive and reliable
3. Fixed the back button functionality
4. Improved the settings mode stability
5. Added proper state management throughout the system
6. Fixed button test mode interference
7. Improved pin assignments for easier wiring

### Notes
- The encoder can use any digital I/O pins on the RP2040
- All GPIO pins support interrupts for the encoder
- Internal pull-ups are used for all buttons and encoder
- I2C pins (GP0-GP1) are used for the display 