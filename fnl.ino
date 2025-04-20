#include <ArduinoBLE.h>

// Pins
const int buttonPin = 7;
const int trigPin = 9;
const int echoPin = 10;
const int lightPin = A0;

// BLE UUIDs
const char* SERVICE_UUID           = "4f0c973c-4a93-4949-b1bc-58940bb33b45";
const char* SWITCH_CHAR_UUID       = "4f0c973c-4a93-4949-b1bc-58940bb33b46";
const char* DISTANCE_CHAR_UUID     = "4f0c973c-4a93-4949-b1bc-58940bb33b47";
const char* LIGHT_CHAR_UUID        = "4f0c973c-4a93-4949-b1bc-58940bb33b48";

// States
int oldButtonState = LOW;
byte distance;
byte light;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(buttonPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  Serial.println("BLE Central Started");
  BLE.scanForUuid(SERVICE_UUID);
}

void loop() {
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    Serial.print("Found ");
    Serial.println(peripheral.localName());

    if (peripheral.localName() != "LED") return;

    BLE.stopScan();
    controlPeripheral(peripheral);
    BLE.scanForUuid(SERVICE_UUID);
  }
}

void controlPeripheral(BLEDevice peripheral) {
  Serial.println("Connecting...");

  if (!peripheral.connect()) {
    Serial.println("Connection failed");
    return;
  }

  Serial.println("Connected. Discovering attributes...");
  if (!peripheral.discoverAttributes()) {
    Serial.println("Discovery failed");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic switchChar = peripheral.characteristic(SWITCH_CHAR_UUID);
  BLECharacteristic distChar   = peripheral.characteristic(DISTANCE_CHAR_UUID);
  BLECharacteristic lightChar  = peripheral.characteristic(LIGHT_CHAR_UUID);

  if (!switchChar || !distChar || !lightChar) {
    Serial.println("One or more characteristics not found!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    int buttonState = digitalRead(buttonPin);

    if (buttonState != oldButtonState) {
      oldButtonState = buttonState;
      byte value = (byte)(buttonState ? 1 : 0);
      switchChar.writeValue(value);
      Serial.println(buttonState ? "Button Pressed" : "Button Released");
    }

    measureDistance();
    distChar.writeValue(distance);

    measureLight();
    lightChar.writeValue(light);

    delay(300);
  }

  Serial.println("Peripheral disconnected");
}

void measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout added
  Serial.print("Raw duration: ");
  Serial.println(duration);

  if (duration == 0) {
    Serial.println("No echo. Object too far or not detected.");
    distance = 0;
  } else {
    float cm = (duration * 0.034) / 2;
    distance = (byte)cm;
    Serial.print("Distance: ");
    Serial.print(cm);
    Serial.println(" cm");
  }
}

void measureLight() {
  int analogVal = analogRead(lightPin);
  light = (byte)(analogVal > 500 ? 1 : 0);

  Serial.print("Analog Light: ");
  Serial.println(analogVal);
  Serial.print("Light: ");
  Serial.println(light);
}
