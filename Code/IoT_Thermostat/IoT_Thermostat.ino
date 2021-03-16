/*
   Evaluatie Week 5: een op afstand bedienbare IoT centrale verwarmingsregelaar:
   Gebruik een digitale temperatuursensor BME280.
   Geef de waarde weer op je telefoon.
   Creeer een thermometer wijzerplaat instelling op je telefoon.
   Als de temperatuur op de locatie op afstand onder deze vooraf ingestelde waarde komt,
   moet een LED aangaan (het verwarmingssysteem) en dit moet op je telefoon worden weergegeven.
*/

#define BLYNK_PRINT Serial

#include <Wire.h> // Library for I2C Communication
#include <Adafruit_Sensor.h> // Unified Adafruit Sensor abstraction layer Library
#include <Adafruit_BME280.h> // Library for BME280 Sensor
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h> // Library for BLYNK
#include <Adafruit_SSD1306.h> // Library for OLED
#include <Fonts/FreeSerif9pt7b.h> // Library for Fonts / OLED

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Push buttons
int upButtonPin = 5; // Controls the temp in the upward direction
int downButtonPin = 2; // Controls the temp in the downward direction
int upVal; // Status of Up-Button
int downVal; // Status of Down-Button
float setTemp = 20.00; // Starting temperature
String msg = "Down-button state is ";
String msg2 = "  Up-Button state is: ";
int dt = 100; // Delay time, only when pressing a button

// Define RedLED
#define led 26


// Own Blynk Server and port
char server[] = "192.168.10.122"; // RPi4B
#define port 8080 // Default port

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "thB1QKMbkTMO1uV9u1uWwQ-k9PvzaygG";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "BLYNK"; // Use the Orange modem Wi-Fi
char pass[] = "**********";

// Create an object "bme" for I2C communication
Adafruit_BME280 bme;

// Functions to read and store temperature, pressure, altitude and humidity
// This code is telling the ESP32 that every time Blynk app sends a _READ request on a Virtual Pin,
// the ESP32 will read the sensor on that Virtual Pin and then write the sensor reading back to Blynk app.

BLYNK_READ(V1) // on Virtual Pin 1
{
  float temperature = bme.readTemperature(); // Read temperature
  Blynk.virtualWrite(V1, temperature); // Send to Blynk
}

// Virtueele LED in BLYNK
WidgetLED led1(V2); //virtual pin 2

// This function will be called every time Temp. Slider Widget
// in Blynk app writes values to the Virtual Pin V3
BLYNK_WRITE(V3)
{
  setTemp = param.asFloat();; // assigning incoming value from pin V1 to a float variable
  // process received value
}


void setup() {

  // Start the Serial Monitor @ 115200 Baud rate
  Serial.begin(115200);
  Serial.println(F("BME280 test"));
  delay(10);

  // Set the push-buttons as INPUT (Pull-UP in hardware)
  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);

  // Set Red LED as OUTPUT
  pinMode(led, OUTPUT);

  // Because the OLED and the BME280 have different addresses, we can use the same I2C SDA and SCL lines without any problem.
  // The OLED display address is 0x3C and the BME280 address is 0x76.
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Initialize the BLYNK Client
  Blynk.begin(auth, ssid, pass, server, port);

  // We initialize the BME280 sensor with the default 0x76 address.
  bool status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Display the OLED start screen
  display.invertDisplay(true);
  display.setFont(&FreeSerif9pt7b);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 25);
  display.println("IoT\nThermometer");
  display.display();
  delay(100);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(2000);
}

void loop() {

  upVal = digitalRead(upButtonPin);
  downVal = digitalRead(downButtonPin);
  // Serial.print(msg + downVal);
  // Serial.println(msg2 + upVal);

  if (upVal == 1 && downVal == 0) { // When pressing the Up-Button
    setTemp += 0.50;
    delay(dt); // Delay time, only when pressing the button
    if (setTemp > 30.00) {
      setTemp = 30;
    }
  }
  if (upVal == 0 && downVal == 1) { // When pressing the Down-Button
    setTemp -= 0.50;
    delay(dt); // Delay time, only when pressing the button
    if (setTemp < 0.00) {
      setTemp = 0.00;
    }
  }

  // Control hardware LED and BLYNK Virtual LED
  // Chauffage will turn on if setTemp is lower than actual temp.   
  if (bme.readTemperature() < setTemp) {
    digitalWrite(led, HIGH);
    led1.on();
  }
  else {
    digitalWrite(led, LOW);
    led1.off();
  }

  // Clear the OLED buffer
  display.clearDisplay();
  // display current temperature in °C
  display.invertDisplay(false);
  display.setFont();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Current Temperature:");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(String(bme.readTemperature()));
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  // display set temperature in °C
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Set Temperature:");
  display.setTextSize(2);
  display.setCursor(0, 50);
  display.print(setTemp);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // "display.display()" is NOT necessary after every single drawing command,
  // you can batch up a bunch of drawing operations and then update the screen
  // all at once by calling "display.display()".
  display.display();

  // Run the Function to read and print temperature, pressure, altitude and humidity
  Blynk.run();
}
