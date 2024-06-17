#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display I2C address
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C

#define analogPin A0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ;
  }

  // Initialize the display with the correct parameters
  if(!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  drawText("HELLO!");
}

void drawText(String s)
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0,0);

  display.print(s);
}

int smoothedValue = 0;
float alpha = 0.4;

void loop()
{
  if (Serial.available() > 0) {
    // Read the incoming string
    String incomingString = Serial.readString();
    drawText(incomingString);
    Serial.println("Data recieved! String: "+incomingString);
  }

  display.display();
}