# LedSuit-Control

# Setup
This code should compile for the Sparkfun ESP8266 Thing.
And is used to control a WS2812B Ledstrip (75 LEDS), which in our case was sewed in a LED-Suit

# Logic
// Setup
- create WiFi Hotspot
- run Webserver ( To fill in WiFi credentials )
- connect to WiFi
- sync clock with NTP server

// Control
- get status from webserver
- load scenario from webserver
- parse scenario to 3d array
- use array to control the suit

```c
byte example_parsed_scenario [PARTS][EVENTS][6] = {
  {{1,1,255,255,255,50},{1,1,255,0,255,50},{1,1,0,255,0,50},{1,1,0,0,255,50}},
  {{2,1,255,255,255,50},{2,1,0,255,255,50},{2,1,255,255,255,50}},
  {{3,1,255,255,255,50},{3,1,255,0,255,50},{3,1,0,255,0,50},{3,1,0,255,0,50}},
  {{4,1,255,255,255,50},{4,1,0,255,255,50},{4,1,0,255,255,50},{4,1,255,0,255,50},{4,1,0,255,0,50}},
  {{5,1,255,255,255,50},{5,1,0,255,255,50},{5,1,0,255,255,50},{5,1,255,0,255,50},{5,1,0,255,0,50}}};
```
{ PART_ID, MODE_ID, RED_VALUE, GREEN_VALUE, BLUE_VALUE, DELAY_VALUE }

Note: The program doesn't work. However it contains most parts to get it fully working.
