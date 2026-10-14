// Store the number of LEDs controlled independently.
const byte LED_COUNT = 3;
// Assign each LED its own UNO digital output pin.
const byte LED_PINS[LED_COUNT] = {7, 6, 5};
// Set each LED's time between state changes in milliseconds.
const unsigned long TOGGLE_MS[LED_COUNT] = {500, 750, 1000};
// Store a separate timing reference for each LED.
unsigned long lastToggle[LED_COUNT] = {0, 0, 0};
// Track each LED's on/off state, starting with all LEDs off.
bool ledIsOn[LED_COUNT] = {false, false, false};

// Run the initialization once after reset.
void setup() {
  // Visit every LED to initialize its pin.
  for (byte i = 0; i < LED_COUNT; i++) {
    // Configure this LED's pin as an output.
    pinMode(LED_PINS[i], OUTPUT);
    // Start this LED switched off.
    digitalWrite(LED_PINS[i], LOW);
  }
  // Read a common starting time after pin initialization.
  const unsigned long startTime = millis();
  // Visit every LED to initialize its timer.
  for (byte i = 0; i < LED_COUNT; i++) {
    // Start this LED's timer at the common starting time.
    lastToggle[i] = startTime;
  }
}

// Repeat continuously to update all three independent timers.
void loop() {
  // Read the current time once for this pass through the loop.
  const unsigned long now = millis();
  // Check each LED's timer without pausing the other LEDs.
  for (byte i = 0; i < LED_COUNT; i++) {
    // Use unsigned subtraction to check elapsed time across millis rollover.
    if (now - lastToggle[i] >= TOGGLE_MS[i]) {
      // Advance this timer by its interval to preserve its schedule.
      lastToggle[i] += TOGGLE_MS[i];
      // Change this LED to the opposite on/off state.
      ledIsOn[i] = !ledIsOn[i];
      // Set this LED's output high when on and low when off.
      digitalWrite(LED_PINS[i], ledIsOn[i] ? HIGH : LOW);
    }
  }
}
