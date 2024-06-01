# Audio Mixer Notes

## Hardware Requirements

### Hardware Items
- Arduino Uno R3
- 5pc Logarithmic Sliders
- 5pc OLED Screens

### Pins

---

## Software Requirements

### PC Software
Control audio levels of currently playing audio sessions.
1. Recieve slider levels from Arduino through USB communication.
2. Iterate through currently placing audio sessions.
3. Set audio levels according to sliders.
4. Send data back to Arduino based on currently playing sessions.
	- Process name

### Arduino Software
When the sliders are moved, send data to PC. Get process information from PC.
1. Receive process information from PC.
2. Render text/logo to OLED screen.