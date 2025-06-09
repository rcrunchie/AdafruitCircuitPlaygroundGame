#include <Adafruit_CircuitPlayground.h>

// switch/button variables
const byte switchPin = 7;
const byte rightButtonPin = 5;

volatile bool switchState = 0;
volatile bool rightButtonFlag = 0;

volatile bool colorSelect;

// game variables
volatile int placedIndex = -1; // Last correctly placed LED index
volatile int currentLED = 0;
volatile long lastUpdate = 0;
volatile long interval = 300; // Initial speed

volatile uint32_t selectedColor = CircuitPlayground.colorWheel(0);  // default color
volatile int colorIndex = 0; // keep track of position on the color wheel
volatile bool lastSwitchState = false;
volatile bool selectingColor = false;

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();

  pinMode(switchPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(switchPin), switchISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rightButtonPin), rightButtonISR, FALLING);
}

void loop() {
  unsigned long now = millis();

  // Advance the moving white LED
  if (now - lastUpdate >= interval) 
  {
    lastUpdate = now;

    int prevLED = (currentLED - 1 + 10) % 10;

    // Restore previous LED's color
    if (prevLED <= placedIndex) 
    {
      CircuitPlayground.setPixelColor(prevLED, selectedColor); // green
    } 
    else 
    {
      CircuitPlayground.setPixelColor(prevLED, 0, 0, 0); // off
    }

    // Draw current LED white
    CircuitPlayground.setPixelColor(currentLED, 255, 255, 255); // white

    currentLED = (currentLED + 1) % 10;
  }

  // If player pressed the button
  if (rightButtonFlag) 
  {
    rightButtonFlag = 0;

    // Correct selection: next in sequence
    if (currentLED == (placedIndex + 1) % 10) 
    {
      placedIndex++;

      // Immediately set that LED to green
      CircuitPlayground.setPixelColor(placedIndex, 0, 255, 0);

      // Check for level complete
      if (placedIndex >= 9) 
      {
        delay(300);

        // Flash green to show level up
        for (int i = 0; i < 3; i++) 
        {
          CircuitPlayground.clearPixels();
          delay(150);
          for (int j = 0; j < 10; j++) 
          {
            CircuitPlayground.setPixelColor(j, 0, 255, 0);
          }
          delay(150);
        }

        // Speed up and reset for next level
        placedIndex = -1;
        if (interval > 100) interval -= 50;
        CircuitPlayground.clearPixels();
      }
    } 
    else 
    {
      // Wrong button press: flash red and reset
      for (int i = 0; i < 3; i++) 
      {
        for (int j = 0; j < 10; j++) 
        {
          CircuitPlayground.setPixelColor(j, 255, 0, 0); // red flash
        }
        delay(200);
        CircuitPlayground.clearPixels();
        delay(200);
      }

      // Reset state
      placedIndex = -1;
      interval = 300;
      currentLED = 0;
      lastUpdate = millis();
      CircuitPlayground.clearPixels();
    }
  }

  // color selection code using switch
  bool currentSwitch = switchState;

  if (currentSwitch && !lastSwitchState) 
  {
    // switch just turned on → enter color selection mode
    selectingColor = true;
  }

  if (!currentSwitch && lastSwitchState && selectingColor) 
  {
    // switch just turned off → save color and exit color selection
    selectedColor = CircuitPlayground.colorWheel((colorIndex * 256 / 10) & 255);
    selectingColor = false;
    CircuitPlayground.clearPixels();
  }

  lastSwitchState = currentSwitch;

  // Show color selection animation
  if (selectingColor) 
  {
    int offset = millis() / 100;
    for (int i = 0; i < 10; i++) 
    {
      CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(((i * 256 / 10) + offset) & 255));
    }

    colorIndex = (offset / 2) % 10;
    return; // skip the game while selecting color
  }
  else if (!selectedColor) 
  {
  selectedColor = CircuitPlayground.colorWheel(100); // fallback color
  }

}

void switchISR() {
  switchState = digitalRead(switchPin);
}

void rightButtonISR() {
  rightButtonFlag = 1;
}
