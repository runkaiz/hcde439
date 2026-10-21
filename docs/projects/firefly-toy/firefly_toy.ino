// Name the digital pin that drives the red "resting" LED on or off.
const byte RED_PIN = 7;
// Name the PWM pin that drives the green "sparkle" LED.
const byte GREEN_PIN = 6;
// Name the PWM pin that drives the blue "awake" LED.
const byte BLUE_PIN = 5;
// Name the input pin connected to the push button.
const byte BUTTON_PIN = 2;
// Set how much the PWM brightness changes on each fade step.
const int FADE_STEP = 5;
// Set the pause between green steps so the sparkle pulses quickly.
const int SPARKLE_DELAY_MS = 4;

// Run the initialization once after reset.
void setup() {
  // Configure the red LED pin as an output.
  pinMode(RED_PIN, OUTPUT);
  // Configure the green LED pin as an output.
  pinMode(GREEN_PIN, OUTPUT);
  // Configure the blue LED pin as an output.
  pinMode(BLUE_PIN, OUTPUT);
  // Configure the button pin as an input; the external 10 kΩ resistor pulls it LOW.
  pinMode(BUTTON_PIN, INPUT);
}

// Repeat continuously, choosing the toy's mode from the button.
void loop() {
  // Read the button: HIGH means pressed, LOW means released.
  if (digitalRead(BUTTON_PIN) == HIGH) {
    // Play the awake light show while the button is held.
    sparkle();
  } else {
    // Show the resting glow while the button is released.
    rest();
  }
}

// Glow red steadily with green and blue off.
void rest() {
  // Turn the green sparkle LED off.
  analogWrite(GREEN_PIN, 0);
  // Turn the blue awake LED off.
  analogWrite(BLUE_PIN, 0);
  // Turn the red resting LED fully on.
  digitalWrite(RED_PIN, HIGH);
}

// Turn red off, hold blue on, and pulse green once while the button is held.
void sparkle() {
  // Turn the red resting LED off while the toy is awake.
  digitalWrite(RED_PIN, LOW);
  // Turn the blue awake LED fully on.
  analogWrite(BLUE_PIN, 255);
  // Step the green brightness up from 0 to 255.
  for (int brightness = 0; brightness <= 255; brightness += FADE_STEP) {
    // Set the green LED to the current PWM brightness.
    analogWrite(GREEN_PIN, brightness);
    // Hold this brightness briefly so the pulse is quick but visible.
    delay(SPARKLE_DELAY_MS);
  }
  // Step the green brightness back down from 255 to 0.
  for (int brightness = 255; brightness >= 0; brightness -= FADE_STEP) {
    // Set the green LED to the current PWM brightness.
    analogWrite(GREEN_PIN, brightness);
    // Hold this brightness briefly so the pulse is quick but visible.
    delay(SPARKLE_DELAY_MS);
  }
}
