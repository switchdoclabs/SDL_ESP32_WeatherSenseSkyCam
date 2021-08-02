// SDL_ESP32_WeatherSenseSkyCam
// SwitchDoc Labs June 2021
//
#define TXDEBUG
//#undef TXDEBUG
#define MQTT_DEBUG
#undef DISABLEI2C

// Software version
#define SOFTWAREVERSION 19

// Which SkyCam Protocol
#define SKYCAMPROTOCOL 1

// Defining this means no sleep, just stop wifi, etc.

#define NOSLEEP




int WeatherSenseProtocol = 20;



//MSTACK5
// USB C ESP32
//int SDAPIN = 13;
//int SCLPIN = 4;

// USBCAM
//int SDAPIN = 16;
int SDAPIN = 14;
int SCLPIN = 13;


// Sleep Information
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  50        /* Time ESP32 will go to sleep (in seconds) */
#define DEFAULTCONTRASTDELAY 1000 /* mseconds for delay after camera start (autexposure) */
int time_to_sleep = TIME_TO_SLEEP;
int contrast_delay = DEFAULTCONTRASTDELAY;


// Global Variables
int currentRSSI;
//Local WiFi
int WiFiSetupFlag = 0;
String APssid;
String Wssid;
String WPassword;
IPAddress myConnectedIp;
IPAddress myConnectedGateWay;
IPAddress myConnectedMask;
bool WiFiPresent = false;


long lastSize = 0;

int lastResolution = 0;

long MessageID;

long GNDR_reboots = 0;
#define BLINKPIN 4



String adminPassword;


String MQTT_IP;
int MQTT_PORT;

String myID;

float InsideTemperature;
float InsideHumidity;

float BatteryVoltage;
float BatteryCurrent;
float LoadVoltage;
float LoadCurrent;
float SolarPanelVoltage;
float SolarPanelCurrent;

bool blinkLight = true;
int frame_size = 9;  // FRAMESIZE_SVGA

#include "esp_camera.h"
#include <WiFi.h>
#include "TimeLib.h"
#include <TimeLib.h>
#include "Clock.h"
#include "Utility.h"
#include <WiFi.h>

#ifndef DISABLEI2C
#include "SDL_Arduino_INA3221.h"
#include "XClosedCube_HDC1080.h";

XClosedCube_HDC1080 hdc1080;

SDL_Arduino_INA3221 INA3221;

#endif


// the three channels of the INA3221 named for INA3221 Solar Power Controller channels (www.switchdoc.com)
#define LIPO_BATTERY_CHANNEL 1
#define SOLAR_CELL_CHANNEL 2
#define OUTPUT_CHANNEL 3


bool HDC1080_Present = false;
bool SunAirPlus_Present = false;



// picture storage and buffer
#define DEFAULTSENSORSTRING "0;0;0;0;1;1;0;1;0;0;300;1;0;0;0;1;1;1;0;0;1;0;"

/*
  brightness(s, 0);     // -2 to 2
  contrast(s, 0);       // -2 to 2
  saturation(s, 0);     // -2 to 2
  special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  whitebal(s, 1);       // 0 = disable , 1 = enable
  awb_gain(s, 1);       // 0 = disable , 1 = enable
  wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  aec2(s, 0);           // 0 = disable , 1 = enable
  ae_level(s, 0);       // -2 to 2
  aec_value(s, 300);    // 0 to 1200
  gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  agc_gain(s, 0);       // 0 to 30
  gainceiling(s, (gainceiling_t)0);  // 0 to 6
  bpc(s, 0);            // 0 = disable , 1 = enable
  wpc(s, 1);            // 0 = disable , 1 = enable
  gma(s, 1);        // 0 = disable , 1 = enable
  lenc(s, 1);           // 0 = disable , 1 = enable
  hmirror(s, 0);        // 0 = disable , 1 = enable
  vflip(s, 0);          // 0 = disable , 1 = enable
  dcw(s, 1);            // 0 = disable , 1 = enable
  colorbar(s, 0);       // 0 = disable , 1 = enable
*/

String CurrentSensorSettings;

byte byteBuffer[200]; // contains string to be sent by MQTT

int protocolBufferCount;

#include "Crc16.h"

//Crc 16 library (XModem)
Crc16 crc;


#include <Preferences.h>;
/* create an instance of Preferences library */
Preferences preferences;
#include "SkyCamPreferences.h"

#include "ArduinoJson-v6.14.1.h"

// configure brownout detector - if brownout is detected, go to deep sleep for 1/2 hour.
#include "soc/soc.h"
#include "soc/cpu.h"
#include "soc/rtc_cntl_reg.h"
//#include "driver/rtc_cntl.h"
#include "rtc_cntl.h"

#ifdef CONFIG_BROWNOUT_DET_LVL
#define BROWNOUT_DET_LVL CONFIG_BROWNOUT_DET_LVL
#else
#define BROWNOUT_DET_LVL 5
#endif //CONFIG_BROWNOUT_DET_LVL

#define CONFIG_BROWNOUT_DET_LVL_SEL_5 1

intr_handler_t low_voltage()
{
  Serial.println("Low Voltage Detected - Deep Sleep 1/2 Hour");
#define BROWNOUTSLEEP 1800L
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_enable_timer_wakeup(BROWNOUTSLEEP * uS_TO_S_FACTOR); // ESP32 wakes up every X seconds
  esp_deep_sleep_start();
}

void brownout_init()
{
  REG_WRITE(RTC_CNTL_BROWN_OUT_REG,
            RTC_CNTL_BROWN_OUT_ENA /* Enable BOD */
            | RTC_CNTL_BROWN_OUT_PD_RF_ENA /* Automatically power down RF */
            /* Reset timeout must be set to >1 even if BOR feature is not used */
            | (2 << RTC_CNTL_BROWN_OUT_RST_WAIT_S)
            | (BROWNOUT_DET_LVL << RTC_CNTL_DBROWN_OUT_THRES_S));

  ESP_ERROR_CHECK( rtc_isr_register( (intr_handler_t )low_voltage, NULL, RTC_CNTL_BROWN_OUT_INT_ENA_M) );
  printf("Initialized BOD\n");

  REG_SET_BIT(RTC_CNTL_INT_ENA_REG, RTC_CNTL_BROWN_OUT_INT_ENA_M);
}

#define PWDN_GPIO_NUM 32
void powerUpCamera()
{
  //power up the camera if PWDN pin is defined

  pinMode(PWDN_GPIO_NUM, OUTPUT);
  digitalWrite(PWDN_GPIO_NUM, false);
  Serial.println("Camera Power On");

}

void powerDownCamera()
{
  // power down camera

  pinMode(PWDN_GPIO_NUM, OUTPUT);
  digitalWrite(PWDN_GPIO_NUM, true);
  Serial.println("Camera Power Off");
}

void resetGNDRCamera()
{

  //powerDownCamera();

  GNDR_reboots++;
  Serial.println();
  Serial.println("Rebooting with GNDR reboot (or ESP.restart() w/NOSLEEP)");
  writeGNDR_reboots();
  Serial.flush();

  delay(1000);
#ifdef NOSLEEP

  ESP.restart();

#else

  digitalWrite(14, false);
  pinMode(14, OUTPUT);
#endif


  Serial.println("Reset");

}

uint32_t Freq = 0;
void reducePower()
{

  Serial.println("reducing power");
  Serial.println("disable radio");

  WiFi.mode(WIFI_MODE_NULL);




  setCpuFrequencyMhz(10);

  Serial.println("after");
  Freq = getCpuFrequencyMhz();
  Serial.print("CPU Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getXtalFrequencyMhz();
  Serial.print("XTAL Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getApbFrequency();
  Serial.print("APB Bus Clock Freq = ");
  Serial.print(Freq);
  Serial.println(" Hz");


}

void increasePower()
{


  setCpuFrequencyMhz(240);

  Serial.println("after");
  Freq = getCpuFrequencyMhz();
  Serial.print("CPU Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getXtalFrequencyMhz();
  Serial.print("XTAL Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getApbFrequency();
  Serial.print("APB Bus Clock Freq = ");
  Serial.print(Freq);
  Serial.println(" Hz");

}



void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}
/* Wifi and MQTT Functions */
/* setup Blocks */

#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager

//void configModeCallback (WiFiManager *myWiFiManager)
void configModeCallback ()
{

  Serial.println("Entered AP config mode");

  Serial.println(WiFi.softAPIP());

}

// MQTT

#include "aPubSubClient.h"
WiFiClient espClient;
PubSubClient MQTTclient(espClient);
unsigned long MQTTlastMsg = 0;
#define MQTTMSG_BUFFER_SIZE  (400)
char MQTTmsg[MQTTMSG_BUFFER_SIZE];

#include "MQTTMessages.h"

int sendMQTT(int messageType, String argument);

#include "MQTTFunctions.h"




void MQTTreconnect(bool reboot) {
  // Loop until we're reconnected
  if (!MQTTclient.connected()) {
    int i = 0;
    while (i < 5)
    {
      Serial.print("Attempting MQTT connection...");
      // Create a random client ID
      String clientId = "SKYCAMWireless-";
      clientId += String(random(0xffff), HEX);
      // Attempt to connect
      Serial.print("client name=");
      Serial.println(clientId);


      if (MQTTclient.connect(clientId.c_str()), MQTT_PORT, 120) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        //String Topic = "SKYCAM/" + myID;
        //MQTTclient.publish(Topic.c_str(), "hello world");
        String topic;
        topic = "SKYCAM/" + myID + "/COMMANDS";
        Serial.print("sub to topic=");
        Serial.println(topic);

        MQTTclient.subscribe(topic.c_str());

        break;

      } else {
        Serial.print("failed, rc=");
        Serial.print(MQTTclient.state());
        Serial.println(" try again in 2 seconds");
        // Wait 2 seconds before retrying
        delay(2000);

      }
      i++;
    }
    // check for 5 failures and then reboot
    if ((i >= 5) && (reboot == true))
    {
      if (MQTT_IP != "")   // dont reboot if no MQTT IP yet.
      {
        // Force Exception and reboot

        int j;

        j = 343 / 0;
        Serial.print (j);
      }
    }
  }
}



// rssi

int fetchRSSI()
{

  wifi_ap_record_t wifidata;
  if (esp_wifi_sta_get_ap_info(&wifidata) == 0) {

    return wifidata.rssi;
  }
  return 0;
}

/********************/

#include "ESP32Camera.h"

#include "getLocalAP.h"

void setupWifiandMQTT()
{
  Serial.println("setupWiFiandMQTT");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi Already Connected");
    return;
  }

  // check for reset of preferences
  if (blinkLight)
  {
    pinMode(BLINKPIN, OUTPUT);

    // clear GPIO 0
    //pinMode(0, INPUT);
    // turn GPIO4 on

    blinkLED(1, 300);
  }
  /*
    long endtime;



    endtime = millis() + 3000;
    int taplengthendtime;
    bool wentToOne;
    wentToOne = false;



    int tapcount;
    tapcount = 0;
    blinkLED(1, 300);
    while (millis() < endtime)
    {

    // check for double tap GPIO0 Low  twice

    if (digitalRead(0) == 0)
    {

      taplengthendtime = millis() + 500;
      while (millis() < taplengthendtime)
      {
        delay(200);
        if (digitalRead(0) == 1)
        {
          wentToOne = true;
          tapcount++;
          Serial.print("tap");





          break;
        }


      }

    }

    }



    if (tapcount >= 3)
    {
    Serial.print("tapcount=");
    Serial.println(tapcount);
    resetPreferences();
    delay(1000);
    blinkLED(2, 300);
    }


    // reset preferences if GPIO0 Low, if not go on
  */








  //---------------------
  // Setup WiFi Interface
  //---------------------
  // 3 cases:
  // 1) Use stored SSID/psk
  // 2) Local AP
  // 3) Wifi Fails

  WiFiPresent = false;
  WiFi.persistent(false);
  WiFi.begin();


#define WL_MAC_ADDR_LENGTH 6
  // Append the last two bytes of the MAC (HEX'd) to string to make unique
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  myID = macID;



  // check for SSID

  if (WiFi_SSID.length() != 0)
  {
    // use existing SSID
    Serial.println("Using existing SSID/psk");

    Serial.printf("SSID="); Serial.println(WiFi_SSID);
    Serial.printf("psk="); Serial.println(WiFi_psk);
    WiFi.begin(WiFi_SSID.c_str(), WiFi_psk.c_str());
    //Wait for WiFi to connect to AP
    Serial.println("Waiting for Saved WiFi");
#define WAITFORCONNECT 15
    for (int i = 0; i < WAITFORCONNECT * 2; i++)
    {
      if (WiFi.status() == WL_CONNECTED)
      {

        Serial.println("");
        Serial.println("WiFI Connected.");

        Serial.printf("SSID="); Serial.println(WiFi_SSID);
        Serial.printf("psk="); Serial.println(WiFi_psk);
        WiFiPresent = true;




        break;
      }

      Serial.print(".");
      WiFiPresent = false;
      //blinkPixel(0, 1, red, 300);
      delay(1000);

    }
    Serial.println();

  }



  if (WiFiPresent != true)
  {
#define APTIMEOUTSECONDS 900

    WiFiPresent = localAPGetIP(APTIMEOUTSECONDS);
  }


  if (WiFiPresent == true)
  {

    WiFi_SSID = WiFi.SSID();
    WiFi_psk = WiFi.psk();
  }
  // write out preferences


  if (WiFiPresent == true)
  {
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("WiFi Channel= ");
    Serial.println(WiFi.channel());

    Serial.print("Gateway Address:");
    myConnectedGateWay = WiFi.gatewayIP();
    Serial.println(WiFi.gatewayIP());

    Serial.print("Subnet Mask:");
    myConnectedMask = WiFi.subnetMask();
    Serial.println(WiFi.subnetMask());


    Serial.println("--------");
    Serial.println("MQTT Start");
    Serial.println("--------");
    Serial.print("MQTT_IP=");
    Serial.println(MQTT_IP);
    Serial.print("MQTT_PORT=");
    Serial.println(MQTT_PORT);

    MQTTclient.setServer(MQTT_IP.c_str(), MQTT_PORT);
    MQTTclient.setCallback(MQTTcallback);
    //blinkIPAddress();
    MQTTreconnect(true);
    writePreferences();


  }



  // send debug boot up MQTT message


  currentRSSI = fetchRSSI();
  printf("rssi:%d\r\n", currentRSSI);
  //---------------------
  // End Setup WiFi Interface
  //---------------------



}

void reconnectWiFi()
{

  Serial.println("reconnectWiFi");
}
/* Function Blocks */

void takeCameraPicture()
{
  Serial.println("takeCameraPicture");

  //camera_init();
  setSensorValues();
  delay(1000);
  Serial.println("before take picture");
  camera_fb_t * fb;
  take_picture(fb);


  Serial.println("sendMQTTStats");

  //sendMQTT(MQTTCAMERASTATUS, "");
  //sendMQTT(MQTTTEMPHUM, "");
  sendMQTT(MQTTSOLAR, "");


  MessageID++;

}



void checkForMQTTCommands()
{


  MQTTclient.loop();


}


void setup() {

  Serial.begin(115200);    // TXDEBUGging only

  //brownout_init();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println("****************************");
  Serial.println("SkyCam Remote");
  Serial.print("Version: ");
  Serial.println(SOFTWAREVERSION);
  Serial.println("****************************");


  Serial.println("Initial Computer Frequency");
  Freq = getCpuFrequencyMhz();
  Serial.print("CPU Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getXtalFrequencyMhz();
  Serial.print("XTAL Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getApbFrequency();
  Serial.print("APB Bus Clock Freq = ");
  Serial.print(Freq);
  Serial.println(" Hz");

  Serial.print("Total heap: ");Serial.println(ESP.getHeapSize());
  Serial.print("Free heap: ");Serial.println(ESP.getFreeHeap());
  Serial.print("Total PSRAM: ");Serial.println(ESP.getPsramSize());
  Serial.print("Free PSRAM: ");Serial.println(ESP.getFreePsram());

  readPreferences();

  InsideTemperature = 0.0;
  InsideHumidity = 0.0;


  BatteryVoltage = 0.0;
  BatteryCurrent = 0.0;
  LoadVoltage = 0.0;
  LoadCurrent = 0.0;
  SolarPanelVoltage = 0.0;
  SolarPanelCurrent = 0.0;

  //powerDownCamera();

  //setupWifiandMQTT();

  // Check for HDC1080
  HDC1080_Present = false;
#ifndef DISABLEI2C
  hdc1080.begin(0x40);

  Serial.print("Device ID ID=0x");
  int devID;
  devID = hdc1080.readDeviceId();
  Serial.println(devID, HEX); // 0x5449 ID of Texas Instruments

  if (devID == 0x1050)
  {

    HDC1080_Present = true;
    Serial.println("HDC1080 Present");
    HDC1080_Present = true;
    Serial.print("T=");
    InsideTemperature = hdc1080.readTemperature();
    Serial.print(InsideTemperature);
    Serial.print("C, RH=");
    InsideHumidity = hdc1080.readHumidity();
    Serial.print(InsideHumidity);
    Serial.println("%");
  }
  else
  {
    Serial.println("HDC1080 Not Present");
    HDC1080_Present = false;
  }



  // test for SunAirPlus_Present
  SunAirPlus_Present = false;



  uint16_t MIDNumber;
  INA3221.wireReadRegister((uint8_t)0xFE, &MIDNumber);
  Serial.print(F("Manuf ID:   0x"));
  Serial.print(MIDNumber, HEX);
  Serial.println();
  if (MIDNumber != 0x5449)
  {
    SunAirPlus_Present = false;
    Serial.println(F("SunAirPlus Not Present"));
  }
  else
  {
    SunAirPlus_Present = true;

    BatteryVoltage = INA3221.getBusVoltage_V(LIPO_BATTERY_CHANNEL);
    BatteryCurrent = INA3221.getCurrent_mA(LIPO_BATTERY_CHANNEL);

    SolarPanelVoltage = INA3221.getBusVoltage_V(SOLAR_CELL_CHANNEL);
    SolarPanelCurrent = -INA3221.getCurrent_mA(SOLAR_CELL_CHANNEL);

    Serial.println("");
    Serial.print(F("Battery Voltage:  ")); Serial.print(BatteryVoltage); Serial.println(F(" V"));
    Serial.print(F("Battery Current:       ")); Serial.print(BatteryCurrent); Serial.println(F(" mA"));
    Serial.println("");

    Serial.print(F("Solar Panel Voltage:   ")); Serial.print(SolarPanelVoltage); Serial.println(F(" V"));
    Serial.print(F("Solar Panel Current:   ")); Serial.print(SolarPanelCurrent); Serial.println(F(" mA"));
    Serial.println("");


    LoadVoltage = INA3221.getBusVoltage_V(OUTPUT_CHANNEL);
    LoadCurrent = INA3221.getCurrent_mA(OUTPUT_CHANNEL) * 0.75;



    Serial.print(F("Load Voltage:  ")); Serial.print(LoadVoltage); Serial.println(" V");
    Serial.print(F("Load Current:       ")); Serial.print(LoadCurrent); Serial.println(" mA");


  }

#endif
  print_wakeup_reason(); //Print the wakeup reason for ESP32

  camera_init();

  setupWifiandMQTT();

  if (psramFound()) {
    Serial.println("PSRAM Found");
  }
  else
  {
    Serial.println("PSRAM Not Found");
  }

}

void loop() {

  Serial.println("In Loop");



#ifndef DISABLEI2C
  if (SunAirPlus_Present)
  {



    BatteryVoltage = INA3221.getBusVoltage_V(LIPO_BATTERY_CHANNEL);
    BatteryCurrent = INA3221.getCurrent_mA(LIPO_BATTERY_CHANNEL);

    SolarPanelVoltage = INA3221.getBusVoltage_V(SOLAR_CELL_CHANNEL);
    SolarPanelCurrent = -INA3221.getCurrent_mA(SOLAR_CELL_CHANNEL);

    Serial.println("");
    Serial.print(F("Battery Voltage:  ")); Serial.print(BatteryVoltage); Serial.println(F(" V"));
    Serial.print(F("Battery Current:       ")); Serial.print(BatteryCurrent); Serial.println(F(" mA"));
    Serial.println("");

    Serial.print(F("Solar Panel Voltage:   ")); Serial.print(SolarPanelVoltage); Serial.println(F(" V"));
    Serial.print(F("Solar Panel Current:   ")); Serial.print(SolarPanelCurrent); Serial.println(F(" mA"));
    Serial.println("");


    LoadVoltage = INA3221.getBusVoltage_V(OUTPUT_CHANNEL);
    LoadCurrent = INA3221.getCurrent_mA(OUTPUT_CHANNEL) * 0.75;



    Serial.print(F("Load Voltage:  ")); Serial.print(LoadVoltage); Serial.println(" V");
    Serial.print(F("Load Current:       ")); Serial.print(LoadCurrent); Serial.println(" mA");

  }


  if ( HDC1080_Present)
  {
    Serial.print("T=");
    InsideTemperature = hdc1080.readTemperature();
    Serial.print(InsideTemperature);
    Serial.print("C, RH=");
    InsideHumidity = hdc1080.readHumidity();
    Serial.print(InsideHumidity);
    Serial.println("%");
  }

#endif


  camera_fb_t * fb;
  //reconnectWiFi();
  take_picture(fb);
  setupWifiandMQTT();

  send_picture(fb);



  //takeAndSendCameraPicture();

  // MQTT command receive time
  int endtime;

  endtime = millis() + 1000;



  while (millis() < endtime)
  {
    checkForMQTTCommands();

  }


  MQTTclient.disconnect();
  delay(500);


#ifdef NOSLEEP

  // Pause now
  Serial.println("No Sleep - vTask Delay");

  //delay(TIME_TO_SLEEP * 1000L); // just delay

  reducePower();

  vTaskDelay( (TIME_TO_SLEEP * 1000L) / portTICK_PERIOD_MS);

  increasePower();
#else
  // Sleep code



  Serial.print("Going to sleep now for ");
  Serial.print(time_to_sleep);
  Serial.println(" second(s)");
  Serial.flush();




  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // ESP32 wakes up every X seconds
  //esp_light_sleep_start();

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // ESP32 wakes up every X seconds
  esp_deep_sleep_start();


  Serial.println("Never gets here in sleep");

#endif



}
