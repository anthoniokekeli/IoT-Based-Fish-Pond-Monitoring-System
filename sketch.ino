#define BLYNK_TEMPLATE_ID "TMPL2QyS72-Tk"
#define BLYNK_TEMPLATE_NAME "IoT Based Fish Pond Monitoring System"
#define BLYNK_AUTH_TOKEN "VFErArf2dGpl26sY-_g57Tu7Nth2qHca"

#include <math.h>            
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Wi-Fi Credentials
const char* ssid = "Wokwi-GUEST";
const char* pass = "";

// pH Sensor and LED definitions
const int phPin = A0;         // Analog pin for the pH sensor (potentiometer)
const int redLed   = 16;      // Red LED for pH < 7 (acidic)
const int greenLed = 17;      // Green LED for pH ≈ 7 (neutral)
const int blueLed  = 15;      // Blue LED for pH > 7 (alkaline)
const float tolerance = 0.05; // Tolerance for considering pH "exactly" 7

// Oxygen Sensor definitions
const int o2Pin = 34;          
const float o2VoltageMin = 0.0;
const float o2VoltageMax = 5.0;

// Simulation flags
bool simulateSensor = true;    
bool simulateTemp = true;      
const bool simulateO2 = true;  

const unsigned long simulationPeriod = 10000; // 10-second cycle period

// DS18B20 Temperature Sensor
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
    Serial.begin(115200);
    
    // Configure LED pins
    pinMode(redLed, OUTPUT);
    pinMode(greenLed, OUTPUT);
    pinMode(blueLed, OUTPUT);
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, HIGH);
    digitalWrite(blueLed, HIGH);

    // Initialize Sensors
    sensors.begin();
    
    // Connect to Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
    Blynk.run(); // Run Blynk

    // ---- pH Sensor ----
    float pHValue = readPH();
    displayPH(pHValue);
    controlLEDs(pHValue);

    // ---- Temperature Sensor ----
    float temperatureC = readTemperature();
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");

    // ---- Oxygen Sensor ----
    float o2Level = readOxygen();
    Serial.print("Oxygen Level: ");
    Serial.print(o2Level);
    Serial.println(" %");

    // ✅ **Send Data to Blynk**
    Blynk.virtualWrite(V0, pHValue);      // Send pH value
    Blynk.virtualWrite(V1, temperatureC); // Send Temperature
    Blynk.virtualWrite(V2, o2Level);      // Send Oxygen Level

    delay(1000); // Wait 1 second before next measurement
}

// Reads the analog value, converts it to voltage, then maps it to a pH value (0-14)
float readPH() {
    int sensorValue;
    if (simulateSensor) {
        sensorValue = (int)(512 + 511 * sin(2 * PI * millis() / simulationPeriod));
    } else {
        sensorValue = analogRead(phPin);
    }
    
    float voltage = sensorValue * (5.0 / 1023.0);
    float pHValue = mapFloat(voltage, 0.0, 5.0, 0.0, 14.0);
    return constrain(pHValue, 0.0, 14.0);
}

// Simulated or real temperature readings
float readTemperature() {
    if (simulateTemp) {
        return 20.0 + 5.0 * sin(2 * PI * millis() / simulationPeriod);
    } else {
        sensors.requestTemperatures();
        return sensors.getTempCByIndex(0);
    }
}

// Reads the oxygen sensor value and converts it to percentage
float readOxygen() {
    int sensorValue;
    if (simulateO2) {
        sensorValue = (int)(512 + 511 * sin(2 * PI * millis() / simulationPeriod));
    } else {
        sensorValue = analogRead(o2Pin);
    }
    
    float voltage = sensorValue * (5.0 / 1023.0);
    float o2Percentage = mapFloat(voltage, o2VoltageMin, o2VoltageMax, 0.0, 100.0);
    return constrain(o2Percentage, 0.0, 100.0);
}

// Controls the LEDs based on the pH value
void controlLEDs(float pHValue) {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, HIGH);
    digitalWrite(blueLed, HIGH);
    
    if (pHValue < 7.0 - tolerance) {
        digitalWrite(redLed, LOW);
    } else if (pHValue > 7.0 + tolerance) {
        digitalWrite(blueLed, LOW);
    } else {
        digitalWrite(greenLed, LOW);
    }
}

// Displays the pH value on the Serial Monitor
void displayPH(float pHValue) {
    Serial.print("pH: ");
    Serial.println(pHValue);
}

// Maps a float value from one range to another.
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
