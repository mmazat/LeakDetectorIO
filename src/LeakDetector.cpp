/*************************************************************
  This is a DEMO. You can use it only for development and testing.
  You should open Setting.h and modify General options.

  If you would like to add these features to your product,
  please contact Blynk for Businesses:

                   http://www.blynk.io/

 *************************************************************/
/*************************************
  This code has tested on ESP-01S model, GPIO16 must be manually connected to reset pin to wakeup the cheep
  after the deep sleep cycle (https://www.youtube.com/watch?v=foljGPuLjYk)

  rest pin, and GPIO0 and GPIO2 must be pulled up using 5K resistor to VCC, expriment shown it is much more robut
  when they are pulled up rather than floating

  the INPUT pin is 3 (Rx) needs to pulled down to low to trigger the leak alarm

  Feed no more than 3V as input, otherwise it consumes more power in deep sleep mode

  connecto to AP, http to  192.168.4.1, enter blynk auth and access point information
  -port 443
  -server blynk-cloud.com

  Tested senarios:
  wifi is goes off
  password changes
  -SSID changes OK
  -project get deleted from blynk, OK
  check if can connect after long deep sleep with correct config

 *************************************/

#define USE_ESP01S_BOARD          // For all other ESP8266-based boards -
// see "Custom board configuration" in Settings.h

#define APP_DEBUG        // Comment this out to disable debug prints

//#define BLYNK_SSL_USE_LETSENCRYPT   // Comment this out, if using public Blynk Cloud

#define BLYNK_PRINT Serial

//sleep time between each measurement
#define DEEP_SLEEP_TIME 300e6    //microsec  

//in case connection can not be established, sleep long enough to save battery
#define DEEP_SLEEP_TIME_WATCH_DOG 3600e6 //microsec
#define CONNECTION_TIMEOUT 80  //seconds, time to config the chip

//skip restarting the board after blynk connection error, and let the wa  tch dog sleep the board to save power
#define DONT_RESTART_AFTER_ERROR

#define RX_PORT 3
#define INPUT_PIN RX_PORT

#define MSG_LEAK "Leak Detected."
#define MSG_NO_LEAK "No Leak Detected."
#define MSG_SLEEP "In Deep Sleep."

#include <MyBlynkProvisioningESP8266.h>
#include <MyCommonBlynk.h>

//in case of interuption, Blynk stuck and goes to configure mode
//which consumes a lot of power. watch dog will sleep the chip if connectino
//is not stablished
bool setToConfigMode = false;
void ICACHE_RAM_ATTR watchDog() {

  if (Blynk.connected()) {
    return ;
  }

  int sec_passed = millis() / 1000;

  //no hope to connect go to a long sleep to save power
  if (sec_passed >= 2 * CONNECTION_TIMEOUT)
  {
    Serial.println("Blynk connection watchdog timed out, going to deep sleep");
    ESP.deepSleep(DEEP_SLEEP_TIME_WATCH_DOG);

  } //old settings doesn't work, go to config mode, connnect to the AP (192.168.4.1)
  else if (sec_passed >=  CONNECTION_TIMEOUT)
  {
    if (!setToConfigMode) {
      Serial.println("Blynk can't connect, entering config mode");
      BlynkState::set(MODE_WAIT_CONFIG); //blynk.run will take it from there
      setToConfigMode = true;
    }
  }


}

//  TIM_DIV1 = 0,   //80MHz (80 ticks/us - 104857.588 us max)
//  TIM_DIV16 = 1,  //5MHz (5 ticks/us - 1677721.4 us max)//
//  TIM_DIV256 = 3 //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
void timer_init(void) {
  timer1_isr_init();
  timer1_attachInterrupt(watchDog);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  timer1_write(10e6);
}

void setup() {


  pinMode(INPUT_PIN, FUNCTION_0); // this changes Rx port to be GPIO, Required
  pinMode(INPUT_PIN, INPUT_PULLUP);

  delay(100);
  //http://www.forward.com.au/pfod/ESP8266/GPIOpins/ESP8266_01_pin_magic.html
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

  //setup watch dog in case blynk got stuck
  timer_init();

  BlynkProvisioning.begin();
}


void loop() {

  BlynkProvisioning.run();

  if (!Blynk.connected()){
    return ;
  }

  char leak_detect = 0;
  Blynk.virtualWrite(V1, "\xE2\x8F\xB3", "Checking For Leak ...");
  delay(500);


  for (int i = 0; i < 5; ++i)
  {
    if (digitalRead(INPUT_PIN) == LOW) {
      Serial.println(MSG_LEAK);
      Blynk.notify(MSG_LEAK);

      Blynk.virtualWrite(V1, "\xE2\x8F\xB3", MSG_LEAK);  // Send time to Display Widget
      delay(500);

      //clear the display
      Blynk.virtualWrite(V1, "\xE2\x8F\xB3", "!!!!!!!!!!!!!!");  // Send time to Display Widget

      leak_detect = 1;
    }

    delay(1000);
  }

  if (!leak_detect)
  {
    Blynk.virtualWrite(V1, "\xE2\x8F\xB3", MSG_NO_LEAK);  // Send time to Display Widget
    Serial.println(MSG_NO_LEAK);
    delay(1000); //delay for user to see the message

    String sleep_msg = getDateAndTime() + " " + MSG_SLEEP;
    Blynk.virtualWrite(V1, sleep_msg);  // Send time to Display Widget
    Serial.println(sleep_msg);
    delay(200);// delay to make sure data is sent

    ESP.deepSleep(DEEP_SLEEP_TIME);
  }


}
