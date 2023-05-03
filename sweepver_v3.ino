#include <esp_now.h>
#include <WiFi.h>
#include <FastLED.h>

#define LED_PIN 19
#define NUM_LEDS 12

#define LEFT_SWEEPER 2
#define BOTH_SWEEPER 1
#define RIGHT_SWEEPER 0

#define RIGHT_DIRECTION 0
#define LEFT_DIRECTION 1

#define STOP 0
#define CLEAN 1
#define SWEEP 2
#define IDLE 4 

#define SWEEPER_POS_PIN 13


 
/* User-defind Variables Start */
uint8_t skipCommand = 4;
int sweeperPosition;
int sweeperSelect;
int direction = 0;
int intensity = 0;

CRGB leds[NUM_LEDS];

// Power Button
const int BUTTON_PIN = 15;
const int BUTTON_THRESHOLD = 3000; // button must be held for 5 seconds
const int DEBOUNCE_TIME = 50; // debounce time in milliseconds
static unsigned long lastButtonTime = 0;
static int lastButtonState = HIGH;
int buttonState = 0;
/* User-defind Variables End */
 
// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}
void setSweeper(){
    if(digitalRead(SWEEPER_POS_PIN) == 1){
      sweeperSelect = LEFT_SWEEPER;
      Serial.println("Switch: LEFT");
    }else {
      sweeperSelect = RIGHT_SWEEPER;
      Serial.println("Switch: RIGHT");
    }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *data, int len) {
  skipCommand = *data;
}

void packetInterface(int skipCommand) {
  intensity = skipCommand / 3 ; // 0 for stop , 1 for clean and 2 for sweep
  Serial.println("Intensity:" + intensity);

  sweeperPosition = skipCommand % 3; // 0 for right , 1 for both and 2 for left 
  Serial.println("Sweeper Position:" + sweeperPosition);

  setSweeper();

  lightUpLED(intensity, sweeperPosition);
}

void setIntensity(int intensity){
  switch (intensity){
    case CLEAN: 
      cleanCommand();
      break;
    case SWEEP:
      sweepCommand();
      break;
    default:
      stopCommand();
      break;        
  }
}

void lightUpLED(int intensity, int sweeperPosition) {
  if(sweeperPosition ==  BOTH_SWEEPER || 
     sweeperPosition ==  sweeperSelect) { 
    setIntensity(intensity);
  } 
  else stopCommand();
}

void stopCommand() {
  lightRing(STOP);
  Serial.println("STOP!");
}

void cleanCommand() {
  if(skipCommand != 4) {
    lightRing(CLEAN);
    Serial.println("CLEAN!");
  } 
  else {
    lightRing(IDLE);
    Serial.println("IDLE!");
  }
}

void sweepCommand() {
  lightRing(SWEEP);
  Serial.println("SWEEP");
} 

void checkPowerButton(int buttonState)
{
  // debounce the button
  if (buttonState != lastButtonState) {
    lastButtonTime = millis();
  }

  if (millis() - lastButtonTime >= DEBOUNCE_TIME) {
    if (buttonState == HIGH) { // button is pressed
      unsigned long startTime = millis();

      while (digitalRead(BUTTON_PIN) == HIGH && millis() - startTime < BUTTON_THRESHOLD) {
        // wait for button to be released or threshold to be reached
      }

      if (millis() - startTime >= BUTTON_THRESHOLD) {
        // button was held for 5 seconds or more, go to deep sleep
        lightRing(99);
        Serial.println("Going to sleep now");
        while (digitalRead(BUTTON_PIN) == HIGH){}
        esp_deep_sleep_start();
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,1);
  Serial.println("I am up!");

  WiFi.mode(WIFI_AP);
 
  InitESPNow();

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  pinMode(SWEEPER_POS_PIN, INPUT);
  
  esp_now_register_recv_cb(OnDataRecv);
    Serial.println("registered");
    Serial.println(WiFi.macAddress());

}
 
void loop() {
  buttonState = digitalRead(BUTTON_PIN);

  packetInterface(skipCommand);

  Serial.println(skipCommand);
  Serial.println();
  Serial.println();

  checkPowerButton(buttonState);
  lastButtonState = buttonState;

  delay(10);
}

void lightRing(uint8_t region){
   
  switch (region){
    case 0: //stop
    
      leds[0]= CRGB(50, 0,0);
      leds[1]= CRGB(0, 0,0);
      leds[2]= CRGB(0, 0,0);
      leds[3]= CRGB(50, 0,0);
      leds[4]= CRGB(0, 0,0);
      leds[5]= CRGB(0, 0,0);
      leds[6]= CRGB(50, 0,0);
      leds[7] = CRGB(0, 0,0);
      leds[8] = CRGB(0, 0,0);
      leds[9] = CRGB(50, 0,0);
      leds[10]= CRGB(0, 0,0);
      leds[11]= CRGB(0, 0,0);
      FastLED.show();
      delay(100);
      leds[0]= CRGB(0, 0,0);
      leds[1]= CRGB(0, 0,0);
      leds[2]= CRGB(0, 0,0);
      leds[3]= CRGB(0, 0,0);
      leds[4]= CRGB(0, 0,0);
      leds[5]= CRGB(0, 0,0);
      leds[6]= CRGB(0, 0,0);
      leds[7] = CRGB(0, 0,0);
      leds[8] = CRGB(0, 0,0);
      leds[9] = CRGB(0, 0,0);
      leds[10]= CRGB(0, 0,0);
      leds[11]= CRGB(0, 0,0);
      FastLED.show();
      delay(100);
      break;
    case 1: //clean
      leds[0]= CRGB(0, 0,0);
      leds[1]= CRGB(50,50,0);
      leds[2]= CRGB(50,50,0);
      leds[3]= CRGB(0, 0,0);
      leds[4]= CRGB(50, 50,0);
      leds[5]= CRGB(50, 50,0);
      leds[6]= CRGB(0, 0,0);
      leds[7] = CRGB(50, 50,0);
      leds[8] = CRGB(50, 50,0);
      leds[9] = CRGB(0, 0,0);
      leds[10]= CRGB(50, 50,0);
      leds[11]= CRGB(50, 50,0);
      FastLED.show();
      break;
    case 2: //sweep
      leds[0]= CRGB(0, 50, 0);
      leds[1]= CRGB(0, 50, 0);
      leds[2]= CRGB(0, 50, 0);
      leds[3]= CRGB(0, 50, 0);
      leds[4]= CRGB(0, 50, 0);
      leds[5]= CRGB(0, 50, 0);
      leds[6]= CRGB(0, 50, 0);
      leds[7] = CRGB(0, 50, 0);
      leds[8] = CRGB(0, 50, 0);
      leds[9] = CRGB(0, 50, 0);
      leds[10]= CRGB(0, 50, 0);
      leds[11]= CRGB(0, 50, 0);
      FastLED.show();
      break;
    case 99: //turn off
      leds[0]= CRGB(0, 0, 0);
      leds[1]= CRGB(0, 0, 0);
      leds[2]= CRGB(0, 0, 0);
      leds[3]= CRGB(0, 0, 0);
      leds[4]= CRGB(0, 0, 0);
      leds[5]= CRGB(0, 0, 0);
      leds[6]= CRGB(0, 0, 0);
      leds[7] = CRGB(0, 0, 0);
      leds[8] = CRGB(0, 0, 0);
      leds[9] = CRGB(0, 0, 0);
      leds[10]= CRGB(0, 0, 0);
      leds[11]= CRGB(0, 0, 0);
      FastLED.show();
      break;
    default:
      int delayTime = 50; // Adjust the delay time to change the speed of the loading animation
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[0]= CRGB(0, 0,0);
      leds[1]= CRGB(0, 0,0);
      leds[2]= CRGB(0, 0,0);
      leds[3]= CRGB(0, 0,0);
      leds[4]= CRGB(0, 0,0);
      leds[5]= CRGB(0, 0,0);
      leds[6]= CRGB(0, 0,0);
      leds[7] = CRGB(0, 0,0);
      leds[8] = CRGB(0, 0,0);
      leds[9] = CRGB(0, 0,0);
      leds[10]= CRGB(0, 0,0);
      leds[11]= CRGB(0, 0,0);
      leds[i] = CRGB(0, 0, 50); // Set all LEDs to dark blue
      leds[i].fadeToBlackBy(10); // Add a fading effect to the LEDs
      leds[(i + 1) % NUM_LEDS] = CRGB(50, 50, 50); // Set the next LED to white
      FastLED.show(); // Update the LED strip
      delay(delayTime); // Wait for a short period of time
    }
  }
}