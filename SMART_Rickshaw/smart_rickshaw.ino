#include <LiquidCrystal_I2C.h>   // Library for LCD
#include <ESP8266WiFi.h>         // Library for ESP8266 Wi-Fi module
#include <BlynkSimpleEsp8266.h>  // Library for Blynk
#include <HX711.h>
#include <Wire.h>
#include <Pushbutton.h>

// Blynk authentication token and Wi-Fi credentials
char auth[] = "RXLtKLm6_bvRbLxnnmKsXJvuexN3X8yO";
char ssid[] = "White House";
char pass[] = "@Shahin20#";

// LCD settings
LiquidCrystal_I2C lcd(0x27, 16, 2);

// HX711 circuit wiring
#define LOADCELL_DOUT_PIN D6
#define LOADCELL_SCK_PIN D7

HX711 scale;
int reading;
int lastReading;
#define CALIBRATION_FACTOR 444.46  // REPLACE WITH YOUR CALIBRATION FACTOR

// Button pin
#define BUTTON_PIN 14
Pushbutton button(BUTTON_PIN);

// Rodeometer settings
#define outputA D3  // DT of rodometer
#define outputB D4  // CLK of rodometer

long counter = 0;
int aState;
int aLastState;
float distance = 0;
float meter = 0;
float taka = 0;

void displayWeight(int weight) {
  lcd.setCursor(0, 1);
  lcd.print("Weight:");
  Serial.print(weight);
  //lcd.setCursor(3, 1);
  lcd.print(weight);
  lcd.print("g");
  Blynk.virtualWrite(V3, weight);  // Send 'weight' value to virtual pin V3
  if (weight >= 1000) {
    for (int i = 1; i <= 3; i++) {
      Blynk.virtualWrite(V4, HIGH);
      digitalWrite(D0, HIGH);
      delay(500);
      Blynk.virtualWrite(V4, LOW);
      digitalWrite(D0, LOW);
      delay(500);
    }
  } else {
    Blynk.virtualWrite(V4, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setContrast(0);
  lcd.begin(16, 2);
  pinMode(D0, OUTPUT);
  lcd.setCursor(0, 0);
  lcd.print("MT=");
  lcd.setCursor(9, 0);
  lcd.print("TK=");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Rodeometer setup
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);

  aLastState = digitalRead(outputA);
}

void loop() {
  Blynk.run();

  if (button.getSingleDebouncedPress()) {
    lcd.setCursor(0, 1);
    lcd.print("Please Wait 4 0");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("Taring..        ");
    delay(1000);
    scale.tare();
  }

  aState = digitalRead(outputA);

  // Check if the state of outputA has changed
  if (aState != aLastState) {
    // Check if the outputB state is different from outputA state
    if (digitalRead(outputB) != aState) {
      counter--;  // Decrement counter for anti-clockwise rotation
    } else {
      counter++;  // Increment counter for clockwise rotation
    }
    distance = counter * 1;
    meter = distance / 100;
    taka = meter * 5;

    lcd.setCursor(3, 0);
    lcd.print(meter);

    lcd.setCursor(12, 0);
    lcd.print(taka);

    Blynk.virtualWrite(V1, meter);  // Send 'meter' value to virtual pin V1
    Blynk.virtualWrite(V2, taka);   // Send 'taka' value to virtual pin V2
  }

  aLastState = aState;

  if (scale.wait_ready_timeout(200)) {
    reading = round(scale.get_units());
    if (reading != lastReading) {
      displayWeight(reading);
    }
    lastReading = reading;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Check Connections");
    lcd.setCursor(0, 1);
    lcd.print("HX711 not found.");
    delay(1000);
  }
}
