#ifndef MIXER_CLASS_H
#define MIXER_CLASS_H

class Mixer_Class
{
public:
  String m_title{"None"};
  int m_volume{};

  int m_slider_pin{A0};
  uint8_t m_screen_channel{0};

  Mixer_Class(int pin, int screen_channel)
    : m_slider_pin{pin}
    , m_screen_channel{screen_channel}
  {
  }
};

#endif