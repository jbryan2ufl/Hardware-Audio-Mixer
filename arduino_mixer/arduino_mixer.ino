#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "mixer_class.h"
#include "icon_data.h"

Mixer_Class mixer1{A3, 0};
Mixer_Class mixer2{A2, 1};
Mixer_Class mixer3{A1, 2};
Mixer_Class mixer4{A0, 3};

Mixer_Class* mixers[] = {&mixer1, &mixer2, &mixer3, &mixer4};

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display I2C address
#define OLED_RESET -1

#define TCA_ADDR 0x70
#define SSD1306_I2C_ADDRESS 0x3C

#define analogPin A0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
  Serial.begin(9600);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  Wire.begin();

  for (uint8_t channel = 0; channel < 4; channel++)
  {
    selectChannel(mixers[channel]->m_screen_channel);
    delay(5);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
      // Serial.print("SSD1306 allocation failed");
      // Serial.println(channel);
      while(1);
    }
    display.clearDisplay();
    drawTitle(*mixers[channel]);
    drawVolumePercentage(0);
    drawBitmap();
    display.display();
  }
}

void selectChannel(uint8_t channel)
{
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void drawTitle(Mixer_Class& m)
{
  display.fillRect(0, 0, 128, 16, SSD1306_BLACK);

  display.setCursor(0,-1);

  display.print(m.m_title);
}

void drawVolumePercentage(int x)
{
  display.fillRect(0, 20, 64, 32, SSD1306_BLACK);

  display.setCursor(0, 20);

  display.print(String(x)+"%");
}

int sensitivity = 25;

int slider_value_1{};
int slider_value_2{};
int slider_value_3{};
int slider_value_4{};
int* sliders[] = {&slider_value_1, &slider_value_2, &slider_value_3, &slider_value_4};

void loop()
{
  slider_value_1 = analogRead(mixer1.m_slider_pin);
  slider_value_2 = analogRead(mixer2.m_slider_pin);
  slider_value_3 = analogRead(mixer3.m_slider_pin);
  slider_value_4 = analogRead(mixer4.m_slider_pin);

  for (int i{}; i < 4; i++)
  {
    if (abs(*sliders[i] - mixers[i]->m_volume) > sensitivity)
    {
      mixers[i]->m_volume = *sliders[i];
      int smoothed_value{mixers[i]->m_volume * 0.098f};

      sendSliderChange(*mixers[i]);

      selectChannel(mixers[i]->m_screen_channel);
      delay(5);
      drawTitle(*mixers[i]);
      drawVolumePercentage(smoothed_value);
      display.display();
    }
  }

  recieveAudioSource();
  delay(10);
}

void drawBitmap()
{
  display.drawBitmap(96, 32, image, 32, 32, SSD1306_WHITE);
}

void sendSliderChange(Mixer_Class& m)
{
  Serial.println(String(m.m_screen_channel)+String(m.m_volume));
}

void recieveAudioSource()
{
  if (Serial.available() > 0)
  {
    String receivedMessage=Serial.readStringUntil('\n');
    if (receivedMessage[0]='!')
    {
      int index = receivedMessage[1]-'0';
      String name{};
      for (int i{2}; i < receivedMessage.length(); i++)
      {
        name+=receivedMessage[i];
      }
      mixers[index]->m_title=name;
      selectChannel(mixers[index]->m_screen_channel);
      delay(20);
      drawTitle(*mixers[index]);
      display.display();
    }
  }
}

// void recieveAudioSource()
// {
//   if (Serial.available() > 0)
//   {
//     char byte = Serial.read();
//     if (byte == 0) // 0 means to nothing
//     {
//     }
//     else if (byte == 1) // 1 means new source
//     {
//       char channel = Serial.read();
//       String newName = Serial.readStringUntil('\n');
//       mixers[channel]->m_title=newName;
//       selectChannel(channel);
//       delay(5);
//       drawText(newName);
//       Serial.println("new source added "+newName);
//     }
//     // String receivedMessage=Serial.readStringUntil('\n');
//     // Serial.println(receivedMessage);
//     // if (receivedMessage[0] == '!')
//     // {
//     //   String name{};
//     //   int counter{};
//     //   for (int i{1}; i < receivedMessage.length(); i++)
//     //   {
//     //     if (receivedMessage[i] == ':')
//     //     {
//     //       mixers[counter]->m_title=name;

//     //       selectChannel(counter);
//     //       delay(5);
//     //       drawText(mixers[counter]->m_title);

//     //       counter++;
//     //       name="";
//     //       if (counter == 4)
//     //       {
//     //         return;
//     //       }
//     //     }
//     //     else
//     //     {
//     //       name += receivedMessage[i];
//     //     }
//     //   }
//     // }
//   }
//   // Serial.println();
// }