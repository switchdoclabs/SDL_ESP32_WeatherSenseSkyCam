//
// Use Local AP  (192.168.4.1)
//


bool localAPGetIP(long apTimeOutSeconds)
{

  bool myWiFiPresent;
  myWiFiPresent = false;

  // set up AP point for reading ssid/password since SmartConfig didn't work
  // Set up Wifi


#define WL_MAC_ADDR_LENGTH 6
  // Append the last two bytes of the MAC (HEX'd) to string to make unique
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  APssid = "SkyCam-" + macID;


  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
#ifdef TXDEBUG
  wifiManager.setDebugOutput(true);
#else
  wifiManager.setDebugOutput(false);
#endif
  //reset saved settings
  //wifiManager.resetSettings();



  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  //wifiManager.setAPCallback(configModeCallback);
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  wifiManager.setTimeout(apTimeOutSeconds);
  //and goes into a blocking loop awaiting configuration

  if (!wifiManager.startConfigPortal(APssid.c_str())) {

    Serial.println("failed to connect and hit timeout");



    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    //delay(1000);
    myWiFiPresent = false;
  }

  if (WiFi.status()  == WL_CONNECTED)
  {
    myWiFiPresent = true;

  }


  return myWiFiPresent;
}
