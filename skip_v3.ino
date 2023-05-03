#include <esp_now.h>
#include <WiFi.h>

#define ADC_Resolution 4096 // 12 bit ADC: 2^12 = 4096 levels
#define CPU_Frequency_MHz 80


/* User-defind Variables Start */
// Joystick Variables
const int posX_in_Pin = 36;  // Analog input pin that the Joystick skipCommand x
const int posY_in_Pin = 39;  // Analog input pin that the Joystick skipCommand x
const int analogOutPin = 9;  // Analog output pin that the LED is attached to
int posX_ADC_value = 0;  // value read from the pot
int posY_ADC_value = 0;  // value read from the pot
int dimension_Division = 3; // Dimension

// Skip Command Variables
uint8_t skipCommand;
uint8_t prevSkipCommand = 4;

// Sweepers' Variables
uint8_t LEAD_MAC_Address[] = {0x40, 0x22, 0xD8, 0xEB, 0x0B, 0x65};
uint8_t SECOND_MAC_Address[] = {0x40, 0x22, 0xD8, 0xEB, 0x2C, 0x6D}; // TO-Do: Modify
esp_now_peer_info_t LEAD_Info;
esp_now_peer_info_t SECOND_Info;

// Power Button
const int BUTTON_PIN = 15;
const int BUTTON_THRESHOLD = 3000; // button must be held for 5 seconds
const int DEBOUNCE_TIME = 50; // debounce time in milliseconds
static unsigned long lastButtonTime = 0;
static int lastButtonState = HIGH;
int buttonState = 0;
/* User-defind Variables End */

/*
function:
    Convert raw adc values for the x and y skipCommands of the joystick to a grid skipCommand. The grid
    skipCommand increments across rows, and with increasing columns 

parameters:
    x: the raw adc value for the x skipCommand
    y: the raw adc value for the y skipCommand

return: 
    the grid skipCommand corresponding to the x and y skipCommand.
*/
uint8_t convert_ADC_to_skipCommand(int x, int y){
  int x_pos, y_pos;
  uint8_t pos;
  x_pos = map(x, 0, ADC_Resolution, 0, dimension_Division);
  y_pos = map(y, 0, ADC_Resolution, 0, dimension_Division);
  pos = y_pos + (dimension_Division * x_pos); 
  return pos;
}

int broadcastSkipCommand(uint8_t skipCommand){
  return (esp_now_send(LEAD_Info.peer_addr, &skipCommand, sizeof(uint8_t)) &&
          esp_now_send(SECOND_Info.peer_addr, &skipCommand, sizeof(uint8_t)));
}

void esp_register_sweepers(){
  memcpy(LEAD_Info.peer_addr, LEAD_MAC_Address, 6);
  memcpy(SECOND_Info.peer_addr, SECOND_MAC_Address, 6);

  if (esp_now_add_peer(&LEAD_Info) != ESP_OK ||
      esp_now_add_peer(&SECOND_Info) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
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
        broadcastSkipCommand(4);
        Serial.println("Going to sleep now");
        while (digitalRead(BUTTON_PIN) == HIGH){}
        esp_deep_sleep_start();
      }
    }
  }
}

void setup() {
  setCpuFrequencyMhz(CPU_Frequency_MHz);  
  
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,1);
  Serial.println("I am up!");
 
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_7dBm); 
 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_register_sweepers();
}
 
void loop() {
  // Read the analog in value:
  posX_ADC_value = analogRead(posX_in_Pin);
  posY_ADC_value = analogRead(posY_in_Pin);
  buttonState = digitalRead(BUTTON_PIN);

  // Calculate the Skip Command
  skipCommand = convert_ADC_to_skipCommand(posX_ADC_value, posY_ADC_value);

  // Only send New Commands (Power Save)
  if (skipCommand != 4 && prevSkipCommand != skipCommand){
    // Broadcast to Sweepers
    if (broadcastSkipCommand(skipCommand) == ESP_OK) {
      Serial.print("Sent with success ");
      Serial.println(skipCommand);
    } else {
      Serial.println("Error sending the data");
    }
  }
  else Serial.println("No change! No commands sent!");

  checkPowerButton(buttonState);

  prevSkipCommand = skipCommand;
  lastButtonState = buttonState;

  delay(10);
}
