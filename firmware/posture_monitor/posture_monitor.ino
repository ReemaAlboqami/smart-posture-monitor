/*
 * Smart Posture Monitor
 * Arduino UNO + flex sensor (SEN-08606) + ADXL335 accelerometer + HC-05 Bluetooth
 *
 * Reads the flex sensor and accelerometer, decides whether the wearer is
 * slouching, and sends the readings and status over Bluetooth to a
 * serial terminal app on the phone.
 *
 * Wiring:
 *   Flex sensor pin 1 -> 5V
 *   Flex sensor pin 2 -> A4, and through a 10K resistor to GND
 *   ADXL335 X_OUT     -> A1
 *   ADXL335 Y_OUT     -> A2
 *   ADXL335 Z_OUT     -> A3
 *   HC-05 RXD         -> D4
 *   HC-05 TXD         -> D5
 */

#include <SoftwareSerial.h>

// ---- Pins ----
const int FLEX_PIN = A4;
const int X_PIN    = A1;
const int Y_PIN    = A2;
const int Z_PIN    = A3;
const int BT_RX    = 5;   // Arduino receives on D5 (HC-05 TXD)
const int BT_TX    = 4;   // Arduino sends on D4    (HC-05 RXD)

SoftwareSerial bluetooth(BT_RX, BT_TX);

// ---- Posture settings ----
// Bend is measured as the change from the upright reading taken at startup,
// so upright is 0 and the value grows as the sensor bends.
const int BEND_THRESHOLD = 300;
const int SAMPLES = 10;
const unsigned long REPORT_INTERVAL_MS = 500;

int flexBaseline = 0;
unsigned long lastReport = 0;

int readAverage(int pin) {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(pin);
    delay(2);
  }
  return sum / SAMPLES;
}

void calibrate() {
  // Sit upright and stay still while this runs.
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += readAverage(FLEX_PIN);
    delay(50);
  }
  flexBaseline = sum / 20;
}

void sendLine(const String& line) {
  bluetooth.println(line);
  Serial.println(line);
}

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);   // HC-05 default baud rate

  sendLine("Calibrating. Sit upright...");
  calibrate();
  sendLine("Ready.");
}

void loop() {
  if (millis() - lastReport < REPORT_INTERVAL_MS) return;
  lastReport = millis();

  int bend = abs(flexBaseline - readAverage(FLEX_PIN));
  int x = readAverage(X_PIN);
  int y = readAverage(Y_PIN);
  int z = readAverage(Z_PIN);

  bool slouching = bend > BEND_THRESHOLD;

  sendLine("--- Posture Data ---");
  sendLine("  Flex Sensor : " + String(slouching ? 1 : 0));
  sendLine("  X-Axis      : " + String(x));
  sendLine("  Y-Axis      : " + String(y));
  sendLine("  Z-Axis      : " + String(z));
  sendLine(slouching ? "  STATUS: Bad Posture" : "  STATUS: Good Posture");
  sendLine("");
}
