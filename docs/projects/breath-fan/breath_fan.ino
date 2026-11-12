// Adafruit "DHT sensor library": handles the DHT11's single-wire timing and returns humidity
#include <DHT.h>

// D2 reads the DHT11 module's data pin (S)
const int DHT_PIN = 2;
// D9 is a PWM pin (490 Hz); it drives the MOSFET's gate through a 330 ohm resistor
const int FAN_PIN = 9;

// The fan starts once humidity is this many percentage points above the room baseline
const float START_RISE = 3.0;
// At this many points above the baseline the fan is at full speed
const float FULL_RISE = 25.0;
// The motor is rated 3-6 V but the supply is 12 V, so full speed is capped at 106/255 = 42% duty, about 5 V on average
const int MAX_DUTY = 106;
// Slowest PWM value (15% duty, about 1.8 V on average) that keeps the motor and blade turning; lower values stall it
const int MIN_DUTY = 38;
// A stopped motor needs more than MIN_DUTY to break free, so it gets a short kick at the 5 V maximum
const int KICK_DUTY = MAX_DUTY;
// Length of that kick in milliseconds
const unsigned long KICK_MS = 150;
// Read every 2 seconds, since the DHT11 and its library allow at most one reading per 2 s
const unsigned long READ_MS = 2000;
// Change the PWM value by 1 every 30 ms, so a full MIN_DUTY to MAX_DUTY change takes about 2 seconds
const unsigned long RAMP_MS = 30;
// Number of readings averaged at startup to record the room's humidity (5 readings x 2 s = 10 s)
const int BASELINE_READINGS = 5;

// Sensor object for a DHT11 on D2
DHT dht(DHT_PIN, DHT11);
// Room humidity in percent, recorded once at startup and then fixed; -1 means not yet known
float baseline = -1;
// The PWM value the fan is heading toward
int targetDuty = 0;
// The PWM value currently on D9
int currentDuty = 0;
// The time of the last sensor reading
unsigned long lastReadAt = 0;
// The time of the last one-step speed change
unsigned long lastRampAt = 0;

// Runs once at power-up or reset
void setup() {
  // Open the serial port at 9600 baud to log each reading
  Serial.begin(9600);
  // Until this line D9 is an INPUT (high impedance), so the 10 kohm pull-down holds the gate at 0 V and the motor is off
  // pinMode makes D9 an OUTPUT; its output latch is still 0 from reset, so it drives LOW (0 V) and the motor stays off
  pinMode(FAN_PIN, OUTPUT);
  // Write a duty of 0 anyway, so the fan is off even if this setup() is not following a full reset
  analogWrite(FAN_PIN, 0);
  // Start the sensor; the library sets D2 to INPUT_PULLUP, a high-impedance input with an internal pull-up to 5 V
  dht.begin();

  // Tell the Serial Monitor that calibration has started and to keep breath away from the sensor
  Serial.println("Recording baseline humidity, keep away from the sensor for 10 s");
  // Running total of the good readings
  float total = 0;
  // How many readings were good
  int good = 0;
  // Take BASELINE_READINGS readings with the fan held off
  for (int i = 0; i < BASELINE_READINGS; i++) {
    // Wait out the library's 2 s minimum between readings
    delay(READ_MS);
    // Read the room's humidity in percent
    float humidity = dht.readHumidity();
    // Only count readings the sensor actually answered
    if (!isnan(humidity)) {
      // Add this reading to the total
      total += humidity;
      // Count it
      good++;
    }
  }
  // Use the average as the fixed baseline, if at least one reading worked
  if (good > 0) {
    // Average of the good readings
    baseline = total / good;
  }
  // Label the recorded baseline in the log
  Serial.print("baseline=");
  // Print it to one decimal place (-1 means every reading failed, so the first good one in loop() is used)
  Serial.println(baseline, 1);
  // Start the reading timer now so the next reading is 2 s after calibration
  lastReadAt = millis();
}

// Runs over and over after setup()
void loop() {
  // Use one timestamp for every check in this pass through the loop
  unsigned long now = millis();

  // Take a new reading once every READ_MS
  if (now - lastReadAt >= READ_MS) {
    // Restart the reading timer
    lastReadAt = now;
    // Ask the DHT11 for relative humidity in percent
    float humidity = dht.readHumidity();

    // The library returns NaN when the sensor does not answer (usually a loose wire)
    if (isnan(humidity)) {
      // Report the failure and keep the fan where it is
      Serial.println("DHT11 read failed");
    } else {
      // If calibration failed, fall back to the first good reading as the baseline
      if (baseline < 0) {
        // Use this reading as the baseline
        baseline = humidity;
      }
      // How far above the room's humidity the air at the sensor is
      float rise = humidity - baseline;

      // A small rise is room drift, not a breath
      if (rise < START_RISE) {
        // Turn the fan off
        targetDuty = 0;
      } else {
        // Cap the rise at FULL_RISE, since the fan cannot go above full speed
        float capped = min(rise, FULL_RISE);
        // Scale START_RISE..FULL_RISE onto MIN_DUTY..MAX_DUTY, so more breath means more speed
        targetDuty = MIN_DUTY + (int)((capped - START_RISE) * (MAX_DUTY - MIN_DUTY) / (FULL_RISE - START_RISE));
      }

      // Label the humidity in the log
      Serial.print("humidity=");
      // Print the humidity in percent
      Serial.print(humidity, 0);
      // Label the baseline
      Serial.print(" baseline=");
      // Print the baseline to one decimal place
      Serial.print(baseline, 1);
      // Label the target PWM value
      Serial.print(" target=");
      // Print the PWM value the fan is heading toward
      Serial.println(targetDuty);
    }
  }

  // Change the speed by at most one step every RAMP_MS
  if (now - lastRampAt >= RAMP_MS) {
    // Restart the ramp timer
    lastRampAt = now;

    // The fan is slower than it should be
    if (currentDuty < targetDuty) {
      // A stopped motor needs a kick to start turning
      if (currentDuty == 0) {
        // Maximum allowed power for a moment to overcome the stopped motor's friction
        analogWrite(FAN_PIN, KICK_DUTY);
        // Hold the kick for KICK_MS
        delay(KICK_MS);
        // Drop to the slowest steady speed, then ramp up from there
        currentDuty = MIN_DUTY;
      } else {
        // Speed up by one step
        currentDuty++;
      }
      // Send the new duty to the MOSFET's gate
      analogWrite(FAN_PIN, currentDuty);
    } else if (currentDuty > targetDuty) {
      // Below MIN_DUTY the motor would stall, so from there go straight to off; otherwise slow by one step
      currentDuty = (currentDuty <= MIN_DUTY) ? 0 : currentDuty - 1;
      // Send the new duty to the MOSFET's gate
      analogWrite(FAN_PIN, currentDuty);
    }
  }
}
