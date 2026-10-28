// A0 reads the voltage between the photoresistor (to 5V) and the 10k resistor (to GND)
const int SENSOR_PIN = A0;
// Red LED on D7 is the on/off "too dark" alert (D7 is not a PWM pin)
const int RED_PIN = 7;
// Green LED on D6 (PWM) glows brighter as the room gets brighter
const int GREEN_PIN = 6;
// Blue LED on D5 (PWM) glows brighter as the room gets darker
const int BLUE_PIN = 5;

// Measured with a finger over the sensor: about 122 counts = 0.60 V, so R_photo is about 74k
const int DARK_READING = 120;
// Measured under a phone flashlight: about 1000 counts = 4.89 V, so R_photo is about 230 ohms
const int BRIGHT_READING = 1000;
// Alert below 150 counts = 0.73 V (R_photo about 58k): between covered (~122) and hand shadows (~175)
const int DARK_ALERT_READING = 150;

// Runs once at power-up or reset
void setup() {
  // Open the serial port at 9600 baud so the Serial Monitor can log readings
  Serial.begin(9600);
  // Make the red alert pin an output
  pinMode(RED_PIN, OUTPUT);
  // Make the green PWM pin an output
  pinMode(GREEN_PIN, OUTPUT);
  // Make the blue PWM pin an output
  pinMode(BLUE_PIN, OUTPUT);
}

// Runs over and over after setup()
void loop() {
  // Read the divider voltage as a 10-bit number: 0 = 0 V, 1023 = 5 V
  int lightReading = analogRead(SENSOR_PIN);
  // Convert the reading back to volts for the serial log and the voltage graph
  float senseVolts = lightReading * 5.0 / 1023.0;
  // Clamp to the calibrated range so map() never returns a value outside 0-255
  int clamped = constrain(lightReading, DARK_READING, BRIGHT_READING);
  // Scale the 10-bit light range onto the 8-bit PWM range: dark = 0, bright = 255
  int daylight = map(clamped, DARK_READING, BRIGHT_READING, 0, 255);
  // Scale the same range in reverse for the night light: dark = 255, bright = 0
  int nightlight = map(clamped, DARK_READING, BRIGHT_READING, 255, 0);
  // Set the green LED's PWM duty cycle from the daylight level
  analogWrite(GREEN_PIN, daylight);
  // Set the blue LED's PWM duty cycle from the night-light level
  analogWrite(BLUE_PIN, nightlight);

  // Check whether the room is dimmer than the alert threshold
  bool tooDark = lightReading < DARK_ALERT_READING;
  // If it is too dark, warn that the home's lights should be on
  if (tooDark) {
    // Turn the red alert LED on
    digitalWrite(RED_PIN, HIGH);
  } else {
    // Otherwise keep the red alert LED off
    digitalWrite(RED_PIN, LOW);
  }

  // Start the log line with the time since reset, for graphing voltage over time
  Serial.print("t_ms=");
  // Print the milliseconds since reset
  Serial.print(millis());
  // Label the raw ADC value
  Serial.print(" | A0=");
  // Print the raw 0-1023 reading
  Serial.print(lightReading);
  // Label the voltage
  Serial.print(" | V=");
  // Print the divider voltage with two decimal places
  Serial.print(senseVolts, 2);
  // Label the green and blue PWM levels
  Serial.print(" V | daylight PWM=");
  // Print the green PWM value
  Serial.print(daylight);
  // Separate the blue value
  Serial.print(" night PWM=");
  // Print the blue PWM value
  Serial.print(nightlight);
  // End the line with the alert state in words
  Serial.println(tooDark ? " | ALERT: room too dark, turn lights on" : " | room light OK");

  // Wait 100 ms so the log has 10 samples per second
  delay(100);
}
