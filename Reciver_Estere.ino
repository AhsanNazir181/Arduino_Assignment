#include <ArduinoBLE.h>

// BLE Service and Characteristics
BLEService ledService("4f0c973c-4a93-4949-b1bc-58940bb33b45"); //needed in order to connect to BLE

BLEByteCharacteristic switchCharacteristic("4f0c973c-4a93-4949-b1bc-58940bb33b46", BLERead | BLEWrite); //I need to recive data when button is pressed
BLEByteCharacteristic distanceCharacteristic("4f0c973c-4a93-4949-b1bc-58940bb33b47", BLERead | BLEWrite); //I need to revive information about sistance
BLEByteCharacteristic lightCharacteristic("4f0c973c-4a93-4949-b1bc-58940bb33b48", BLERead | BLEWrite); // Ineed to recive information about light

// Pin assignments
const int buttonLedPin = 13; //assignin LED to pin 13; will be used for button
const int lightLedPin = 8; //assignin LED to pin 8; will be used for light data
const int buzzerPin = 7; ////assignin buzzer to pin 7; will be used for distance data

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  pinMode(buttonLedPin, OUTPUT);
  pinMode(lightLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    while (1);
  }
  
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);
  
  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(distanceCharacteristic);
  ledService.addCharacteristic(lightCharacteristic);
  
  BLE.addService(ledService);
  
  switchCharacteristic.writeValue(0);
  distanceCharacteristic.writeValue(0);
  lightCharacteristic.writeValue(0);
  
  BLE.advertise();
  Serial.println("BLE Peripheral Ready");
}

void loop() {
 
  BLEDevice central = BLE.central();
  
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    
    while (central.connected()) {
      // Button LED; Logic - when button is pressed the LED turns on, when it is pressed again it turns off
      if (switchCharacteristic.written()) {
        byte switchVal = switchCharacteristic.value();
        Serial.print("Switch: ");
        Serial.println(switchVal);
        digitalWrite(buttonLedPin, switchVal ? HIGH : LOW);
      }
      
      // Distance Buzzer; logic if distance from ultrasonic sensor is >10 cm; the buzzer will give annoying sound
      if (distanceCharacteristic.written()) {
        String distanceStr = String(distanceCharacteristic.value());
int distance = distanceStr.toInt(); // converted to usable number

Serial.print("Distance: ");
Serial.println(distance);

if (distance > 10) {
  tone(buzzerPin, 1000); 
} else {
          noTone(buzzerPin);
        }

}
        
  
      
      // Light LED; logig - if I get low vlaue from light sensor then LED will turn off by it self
      if (lightCharacteristic.written()) {
        byte light = lightCharacteristic.value();
        Serial.print("Light: ");
        Serial.println(light);
        digitalWrite(lightLedPin, light == 1 ? HIGH : LOW);
      }
    }
    
    // Making sure all outputs are off when disconnected
    digitalWrite(buzzerPin, LOW);
    Serial.print("Disconnected from: ");
    Serial.println(central.address());
  }
}