#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Pin definitions
#define ENCODER_A_PIN 2    // GP2 on the Pico
#define ENCODER_B_PIN 3    // GP3 on the Pico
#define ENCODER_BUTTON_PIN 4  // GP4 on the Pico
#define BUTTON1_PIN 5      // GP5 on the Pico
#define BUTTON2_PIN 6      // GP6 on the Pico
#define BUTTON3_PIN 7      // GP7 on the Pico
#define ANALOG_IN 26       // ADC0
#define ANALOG_IN2 27      // ADC1

// Menu states
enum MenuState {
    MAIN_MENU,
    OSCILLOSCOPE_MODE,
    SETTINGS_MODE,
    BUTTON_TEST_MODE
};

// Global variables
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RotaryEncoder encoder(ENCODER_A_PIN, ENCODER_B_PIN);
MenuState currentState = MAIN_MENU;
int encoderValue = 0;
int lastEncoderValue = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
unsigned long lastSampleTime = 0;
const int SAMPLE_INTERVAL = 1000; // 1ms between samples
#define BUFFER_SIZE 128
int sampleBuffer[BUFFER_SIZE];
int bufferIndex = 0;
bool oscilloscopeActive = false;  // New flag to track if oscilloscope is running

// Scope settings
struct ScopeSettings {
    int timeScale;      // Time per division (ms)
    int voltageScale;   // Voltage per division
    int triggerLevel;   // Trigger level (0-1023)
    bool triggerEnabled;
    bool showChannel2;  // Whether to show second channel
    int channel2Offset; // Vertical offset for channel 2
} scopeSettings = {
    .timeScale = 1,     // 1ms per division
    .voltageScale = 100,// 100 units per division
    .triggerLevel = 512,// Middle of range
    .triggerEnabled = false,
    .showChannel2 = false,
    .channel2Offset = 20 // Pixels offset for channel 2
};

// Function declarations
void displayMainMenu();
void handleEncoderChange();
void handleEncoderButton();
void checkButtons();
void updateMainMenu();
void updateOscilloscope();
void updateSettings();
void updateButtonTest();

void setup() {
    // Initialize Serial but don't wait for it
    Serial.begin(115200);
    
    // Initialize I2C for Pico
    Wire.begin();
    
    // Try to initialize the display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        // If display fails, just keep trying
        for(;;) {
            if(display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
                break;
            }
        }
    }
    
    // Additional display configuration
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setRotation(0);
    display.display();
    
    // Initialize pins
    pinMode(ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_PIN, INPUT_PULLUP);
    pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);
    pinMode(ANALOG_IN, INPUT);
    pinMode(ANALOG_IN2, INPUT);
    
    // Initial display
    displayMainMenu();
}

void loop() {
    static MenuState lastState = MAIN_MENU;
    static unsigned long lastDebugTime = 0;
    
    // Handle encoder
    encoder.tick();
    handleEncoderChange();
    handleEncoderButton();
    
    // Check buttons
    checkButtons();
    
    // Debug output every second
    if (millis() - lastDebugTime > 1000) {
        Serial.print(F("Current state: "));
        switch(currentState) {
            case MAIN_MENU:
                Serial.println(F("MAIN_MENU"));
                break;
            case OSCILLOSCOPE_MODE:
                Serial.println(F("OSCILLOSCOPE_MODE"));
                break;
            case SETTINGS_MODE:
                Serial.println(F("SETTINGS_MODE"));
                break;
            case BUTTON_TEST_MODE:
                Serial.println(F("BUTTON_TEST_MODE"));
                break;
        }
        lastDebugTime = millis();
    }
    
    // Update display based on current state
    if (currentState != lastState) {
        Serial.print(F("State changed from "));
        switch(lastState) {
            case MAIN_MENU:
                Serial.print(F("MAIN_MENU"));
                break;
            case OSCILLOSCOPE_MODE:
                Serial.print(F("OSCILLOSCOPE_MODE"));
                break;
            case SETTINGS_MODE:
                Serial.print(F("SETTINGS_MODE"));
                break;
            case BUTTON_TEST_MODE:
                Serial.print(F("BUTTON_TEST_MODE"));
                break;
        }
        Serial.print(F(" to "));
        switch(currentState) {
            case MAIN_MENU:
                Serial.println(F("MAIN_MENU"));
                displayMainMenu();
                break;
            case OSCILLOSCOPE_MODE:
                Serial.println(F("OSCILLOSCOPE_MODE"));
                if (oscilloscopeActive) {
                    updateOscilloscope();
                }
                break;
            case SETTINGS_MODE:
                Serial.println(F("SETTINGS_MODE"));
                updateSettings();
                break;
            case BUTTON_TEST_MODE:
                Serial.println(F("BUTTON_TEST_MODE"));
                updateButtonTest();
                break;
        }
        lastState = currentState;
    } else {
        // State hasn't changed, only update if needed
        switch(currentState) {
            case MAIN_MENU:
                // Menu is static, no need to update
                break;
            case OSCILLOSCOPE_MODE:
                if (oscilloscopeActive) {
                    updateOscilloscope();
                }
                break;
            case SETTINGS_MODE:
                updateSettings();
                break;
            case BUTTON_TEST_MODE:
                updateButtonTest();
                break;
        }
    }
}

void displayMainMenu() {
    Serial.println(F("Updating main menu display"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Test Bed Menu"));
    display.println(F("-------------"));
    
    // Show selection arrow based on encoderValue
    switch(encoderValue) {
        case 0:
            display.println(F("> Oscilloscope"));
            display.println(F("  Settings"));
            display.println(F("  Button Test"));
            break;
        case 1:
            display.println(F("  Oscilloscope"));
            display.println(F("> Settings"));
            display.println(F("  Button Test"));
            break;
        case 2:
            display.println(F("  Oscilloscope"));
            display.println(F("  Settings"));
            display.println(F("> Button Test"));
            break;
    }
    
    // Add pin information at the bottom
    display.setCursor(0, 48);
    display.println(F("Encoder: A="));
    display.print(ENCODER_A_PIN);
    display.print(F(" B="));
    display.print(ENCODER_B_PIN);
    display.print(F(" BTN="));
    display.print(ENCODER_BUTTON_PIN);
    
    display.display();
    Serial.println(F("Main menu display updated"));
}

void handleEncoderChange() {
    int newValue = encoder.getPosition();
    if (newValue != lastEncoderValue) {
        // Determine direction
        int direction = newValue > lastEncoderValue ? 1 : -1;
        
        // Limit the rate of change to prevent overwhelming the system
        static unsigned long lastEncoderUpdate = 0;
        if (millis() - lastEncoderUpdate < 5) {  // Reduced from 10ms to 5ms for better responsiveness
            return;
        }
        lastEncoderUpdate = millis();
        
        Serial.print(F("Encoder moved: "));
        Serial.print(direction > 0 ? F("clockwise") : F("counter-clockwise"));
        Serial.print(F(" ("));
        Serial.print(newValue);
        Serial.println(F(")"));
        
        lastEncoderValue = newValue;
        
        // Handle different modes
        switch(currentState) {
            case MAIN_MENU:
                // Update encoderValue based on direction
                if (direction > 0) {
                    encoderValue = (encoderValue + 1) % 3;
                } else {
                    encoderValue = (encoderValue + 2) % 3;
                }
                displayMainMenu();
                break;
                
            case SETTINGS_MODE:
                // First handle selection
                if (direction > 0) {
                    encoderValue = (encoderValue + 1) % 6;  // Now 6 settings
                } else {
                    encoderValue = (encoderValue + 5) % 6;  // Move backward
                }
                
                // Then handle value changes with rate limiting
                switch(encoderValue) {
                    case 0: // Time scale
                        scopeSettings.timeScale = max(1, min(100, scopeSettings.timeScale + direction));
                        break;
                    case 1: // Voltage scale
                        scopeSettings.voltageScale = max(50, min(500, scopeSettings.voltageScale + direction * 50));
                        break;
                    case 2: // Trigger enable
                        if (direction != 0) {  // Only toggle on actual movement
                            scopeSettings.triggerEnabled = !scopeSettings.triggerEnabled;
                        }
                        break;
                    case 3: // Trigger level
                        scopeSettings.triggerLevel = max(0, min(1023, scopeSettings.triggerLevel + direction * 50));
                        break;
                    case 4: // Channel 2 enable
                        if (direction != 0) {  // Only toggle on actual movement
                            scopeSettings.showChannel2 = !scopeSettings.showChannel2;
                        }
                        break;
                    case 5: // Channel 2 offset
                        scopeSettings.channel2Offset = max(0, min(40, scopeSettings.channel2Offset + direction));
                        break;
                }
                updateSettings();
                break;
                
            case OSCILLOSCOPE_MODE:
                // Adjust channel 2 offset in oscilloscope mode
                if (scopeSettings.showChannel2) {
                    scopeSettings.channel2Offset = max(0, min(40, scopeSettings.channel2Offset + direction));
                }
                break;
                
            case BUTTON_TEST_MODE:
                // No encoder action in button test mode
                break;
        }
    }
}

void handleEncoderButton() {
    static bool buttonPressed = false;
    bool currentButtonState = digitalRead(ENCODER_BUTTON_PIN);
    
    if (currentButtonState != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentButtonState == LOW && !buttonPressed) { // Button just pressed
            buttonPressed = true;
            Serial.println(F("Encoder button pressed"));
            
            switch(currentState) {
                case MAIN_MENU:
                    Serial.print(F("Main menu selection: "));
                    Serial.println(encoderValue);
                    switch(encoderValue) {
                        case 0:
                            currentState = OSCILLOSCOPE_MODE;
                            oscilloscopeActive = true;
                            Serial.println(F("Entering oscilloscope mode"));
                            break;
                        case 1:
                            currentState = SETTINGS_MODE;
                            encoderValue = 0;
                            Serial.println(F("Entering settings mode"));
                            break;
                        case 2:
                            currentState = BUTTON_TEST_MODE;
                            Serial.println(F("Entering button test mode"));
                            break;
                    }
                    break;
                case OSCILLOSCOPE_MODE:
                case SETTINGS_MODE:
                case BUTTON_TEST_MODE:
                    Serial.println(F("Returning to main menu"));
                    if (currentState == OSCILLOSCOPE_MODE) {
                        oscilloscopeActive = false;
                    }
                    currentState = MAIN_MENU;
                    encoderValue = 0;
                    break;
            }
        } else if (currentButtonState == HIGH) {
            buttonPressed = false;
        }
    }
    
    lastButtonState = currentButtonState;
}

void checkButtons() {
    static bool lastButtonState = HIGH;
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;  // Same as encoder button
    
    // Skip button check if in button test mode
    if (currentState == BUTTON_TEST_MODE) {
        return;
    }
    
    bool currentButtonState = (digitalRead(BUTTON1_PIN) == LOW || 
                             digitalRead(BUTTON2_PIN) == LOW || 
                             digitalRead(BUTTON3_PIN) == LOW);
    
    // Check if the button state has changed
    if (currentButtonState != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    // If the button state has been stable for the debounce period
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentButtonState == LOW && lastButtonState == HIGH) {  // Button just pressed
            Serial.println(F("Back button pressed"));
            if (currentState == OSCILLOSCOPE_MODE) {
                oscilloscopeActive = false;
            }
            currentState = MAIN_MENU;
            encoderValue = 0;
            displayMainMenu();  // Update display when returning to menu
        }
    }
    
    lastButtonState = currentButtonState;
}

void updateMainMenu() {
    // Menu is updated in displayMainMenu()
}

void updateOscilloscope() {
    // Only process if oscilloscope is active
    if (!oscilloscopeActive) {
        return;
    }

    if (millis() - lastSampleTime >= SAMPLE_INTERVAL) {
        // Read and store the new values
        int value1 = analogRead(ANALOG_IN);
        int value2 = analogRead(ANALOG_IN2);
        
        // Check trigger if enabled
        if (scopeSettings.triggerEnabled) {
            if (value1 < scopeSettings.triggerLevel) {
                // Wait for trigger
                return;
            }
        }
        
        sampleBuffer[bufferIndex] = value1;
        bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
        
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(F("Oscilloscope"));
        display.println(F("-------------"));
        
        // Draw the waveform for channel 1
        for(int i = 0; i < BUFFER_SIZE - 1; i++) {
            int currentIndex = (bufferIndex + i) % BUFFER_SIZE;
            int nextIndex = (bufferIndex + i + 1) % BUFFER_SIZE;
            
            // Map the values to screen coordinates
            int y1 = map(sampleBuffer[currentIndex], 0, 1023, 48, 0);
            int y2 = map(sampleBuffer[nextIndex], 0, 1023, 48, 0);
            
            // Draw a line between points
            display.drawLine(i, y1, i + 1, y2, SSD1306_WHITE);
        }
        
        // Draw channel 2 if enabled
        if (scopeSettings.showChannel2) {
            for(int i = 0; i < BUFFER_SIZE - 1; i++) {
                int currentIndex = (bufferIndex + i) % BUFFER_SIZE;
                int nextIndex = (bufferIndex + i + 1) % BUFFER_SIZE;
                
                // Map the values to screen coordinates with offset
                int y1 = map(value2, 0, 1023, 48, 0) + scopeSettings.channel2Offset;
                int y2 = map(value2, 0, 1023, 48, 0) + scopeSettings.channel2Offset;
                
                // Draw a line between points
                display.drawLine(i, y1, i + 1, y2, SSD1306_WHITE);
            }
        }
        
        // Show the current values and offset
        display.setCursor(0, 56);
        display.print(F("CH1:"));
        display.print(value1);
        if (scopeSettings.showChannel2) {
            display.print(F(" CH2:"));
            display.print(value2);
            display.print(F(" Off:"));
            display.print(scopeSettings.channel2Offset);
        }
        
        // Add back to menu message
        display.setCursor(0, 0);
        display.print(F("Press any button"));
        display.setCursor(0, 8);
        display.print(F("to return to menu"));
        
        display.display();
        lastSampleTime = millis();
    }
}

void updateSettings() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Scope Settings"));
    display.println(F("-------------"));
    
    // Show current settings with selection indicator
    display.print(encoderValue == 0 ? F(">") : F(" "));
    display.print(F("Time: "));
    display.print(scopeSettings.timeScale);
    display.println(F("ms/div"));
    
    display.print(encoderValue == 1 ? F(">") : F(" "));
    display.print(F("Volt: "));
    display.print(scopeSettings.voltageScale);
    display.println(F("/div"));
    
    display.print(encoderValue == 2 ? F(">") : F(" "));
    display.print(F("Trig: "));
    display.println(scopeSettings.triggerEnabled ? F("ON") : F("OFF"));
    
    display.print(encoderValue == 3 ? F(">") : F(" "));
    display.print(F("Trig Lvl: "));
    display.print(map(scopeSettings.triggerLevel, 0, 1023, 0, 33));
    display.println(F("V"));
    
    display.print(encoderValue == 4 ? F(">") : F(" "));
    display.print(F("CH2: "));
    display.println(scopeSettings.showChannel2 ? F("ON") : F("OFF"));
    
    display.print(encoderValue == 5 ? F(">") : F(" "));
    display.print(F("CH2 Off: "));
    display.print(scopeSettings.channel2Offset);
    display.println(F("px"));
    
    // Add back to menu message at the bottom
    display.setCursor(0, 56);
    display.print(F("Press any button to menu"));
    
    display.display();
}

void updateButtonTest() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Button Test"));
    display.println(F("-------------"));
    
    // Display button states with pin numbers
    display.print(F("Encoder: "));
    display.print(digitalRead(ENCODER_BUTTON_PIN) ? F("UP") : F("DOWN"));
    display.print(F(" (Pin "));
    display.print(ENCODER_BUTTON_PIN);
    display.println(F(")"));
    
    display.print(F("Button 1: "));
    display.print(digitalRead(BUTTON1_PIN) ? F("UP") : F("DOWN"));
    display.print(F(" (Pin "));
    display.print(BUTTON1_PIN);
    display.println(F(")"));
    
    display.print(F("Button 2: "));
    display.print(digitalRead(BUTTON2_PIN) ? F("UP") : F("DOWN"));
    display.print(F(" (Pin "));
    display.print(BUTTON2_PIN);
    display.println(F(")"));
    
    display.print(F("Button 3: "));
    display.print(digitalRead(BUTTON3_PIN) ? F("UP") : F("DOWN"));
    display.print(F(" (Pin "));
    display.print(BUTTON3_PIN);
    display.println(F(")"));
    
    // Add back to menu message
    display.setCursor(0, 56);
    display.print(F("Press encoder to exit"));
    
    display.display();
} 