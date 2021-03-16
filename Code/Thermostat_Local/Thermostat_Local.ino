#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_BME280 bme;

// Push buttons
int upButtonPin = 5; // Controls the temp in the upward direction
int downButtonPin = 2; // Controls the temp in the downward direction
int upVal; // Status of Up-Button
int downVal; // Status of Down-Button
float setTemp = 15.00; // Starting temperature
String msg = "Down-button state is ";
String msg2 = "  Up-Button state is: ";
int dt = 100; // Delay time, only when pressing a button

// Define RedLED
#define led 26

void setup() {

  // Start the Serial Monitor @ 115200 Baud rate
  Serial.begin(115200);

  // Set the push-buttons as INPUT (Pull-UP in hardware)
  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);

  // Set Red LED as OUTPUT
  pinMode(led, OUTPUT);

  // Because the OLED and the BME280 have different addresses, we can use the same I2C SDA and SCL lines without any problem. 
  // The OLED display address is 0x3C and the BME280 address is 0x76.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
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
  display.setCursor(0,25);             
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

  if (bme.readTemperature() <= setTemp){
    digitalWrite(led, HIGH);
  }
  else{
    digitalWrite(led, LOW);
  }
  
  // Clear the OLED buffer
  display.clearDisplay();
  // display current temperature in °C
  display.invertDisplay(false);
  display.setFont();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Current Temperature:");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(String(bme.readTemperature()));
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  // display set temperature in °C
  display.setTextSize(1);
  display.setCursor(0,40);
  display.print("Set Temperature:");
  display.setTextSize(2);
  display.setCursor(0,50);
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
}
