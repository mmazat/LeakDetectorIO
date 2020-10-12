/*************************************************************
  This is a DEMO. You can use it only for development and testing.
  You should open Setting.h and modify General options.

  If you would like to add these features to your product,
  please contact Blynk for Businesses:

                   http://www.blynk.io/

 *************************************************************/

#define USE_ESP01S_BOARD // For all other ESP8266-based boards -
// see "Custom board configuration" in Settings.h

#define APP_DEBUG // Comment this out to disable debug prints

//#define BLYNK_SSL_USE_LETSENCRYPT   // Comment this out, if using public Blynk Cloud

#define BLYNK_PRINT Serial

//sleep time between each measurement
//in case connection can not be established, sleep long enough to save battery

#define WATCH_DOG_TIMEOUT 20 //seconds, time to config the chip

//skip restarting the board after blynk connection error, and let the wa  tch dog sleep the board to save power
#define DONT_RESTART_AFTER_ERROR

#define RX_PORT 3
#define TX_PORT 1
#define INPUT_PIN RX_PORT
#define PIN0 0
#define PIN2 2

#define MSG_LEAK "Leak Detected."
#define MSG_SLEEP "No Leak, sleeping."

#include <MyBlynkProvisioningESP8266.h>
#include <MyCommonBlynk.h>

//in case of interuption, Blynk stuck and goes to configure mode
//which consumes a lot of power. watch dog will sleep the chip if connectino
//is not stablished
bool configMode = false;
bool leak_detected = false;

void ICACHE_RAM_ATTR watchDog()
{

  if (configMode)
  {
    return;
  }

  int sec_passed = millis() / 1000;

  if (sec_passed >= WATCH_DOG_TIMEOUT)
  {
    if (leak_detected)
    {
      ESP.restart();
    }
    else
    {
      ESP.deepSleep(ESP.deepSleepMax());
    }
  }
}

//  TIM_DIV1 = 0,   //80MHz (80 ticks/us - 104857.588 us max)
//  TIM_DIV16 = 1,  //5MHz (5 ticks/us - 1677721.4 us max)//
//  TIM_DIV256 = 3 //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
void timer_init(void)
{
  timer1_isr_init();
  timer1_attachInterrupt(watchDog);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  timer1_write(1e6);
}

void detectLeak()
{
  if (digitalRead(INPUT_PIN) == LOW)
  {
    leak_detected = true;
  }
}

void setup()
{
  //couldn't get pin 0 or 2 as GPIO stably, it makes WIFI connections instable, used TX instead

  //https://www.forward.com.au/pfod/ESP8266/GPIOpins/ESP8266_01_pin_magic.html
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

  pinMode(INPUT_PIN, FUNCTION_3); // this is required
  pinMode(INPUT_PIN, OUTPUT);
  digitalWrite(INPUT_PIN, LOW);
  //if config jumper is not connected, it must stay low
  configMode = digitalRead(INPUT_PIN) == HIGH;

  //revert to high
  digitalWrite(INPUT_PIN, HIGH);
  pinMode(INPUT_PIN, INPUT_PULLUP);

  /**  
  * keeps the led on
  * provides two output ports
 */
  pinMode(PIN0, OUTPUT);
  pinMode(PIN2, OUTPUT);

  if (!configMode)
  {
    //setup watch dog in case blynk got stuck
    timer_init();
  }

  /**
   * check for a leak once here, just to change behaviour of watch dog to restart 
   * it means if there is a leak and board can't connect to blynk, restart the board instead of sleep
   * working great!
  */
  detectLeak();

  
  delay(100);

  BlynkProvisioning.begin();
  if (configMode)
  {
    configMode = true;
    BlynkState::set(MODE_WAIT_CONFIG); //blynk.run will take it from there
    Serial.println("Config mode started");
  }
}

int nTry = 1;
void loop()
{

  BlynkProvisioning.run();

  delay(100);

  if (!Blynk.connected())
  {
    return;
  }

  bool leak_detect = false;
  Blynk.virtualWrite(V1, "\xE2\x8F\xB3", "Checking For Leak ... try " + String(nTry));

  detectLeak();

  if (leak_detected)
  {
    Serial.println(MSG_LEAK);
    Blynk.notify(MSG_LEAK);

    Blynk.virtualWrite(V1, "\xE2\x8F\xB3", MSG_LEAK); // Send time to Display Widget
    delay(500);

    //clear the display
    Blynk.virtualWrite(V1, "\xE2\x8F\xB3", "!!!!!!!!!!!!!!"); // Send time to Display Widget
  }

  if (!leak_detect && ++nTry > 4)
  {
    String sleep_msg = getDateAndTime() + " " + MSG_SLEEP;
    Blynk.virtualWrite(V1, sleep_msg); // Send time to Display Widget
    Serial.println(sleep_msg);
    delay(200); // delay to make sure data is sent
    ESP.deepSleep(ESP.deepSleepMax());
  }
}
