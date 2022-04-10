


String WiFi_SSID = "";
String WiFi_psk = "";
int ClockTimeOffsetToUTC = 0;

void resetPreferences()
{
  preferences.begin("SkyCaminit", false);
  preferences.clear();
  preferences.end();


  Serial.println("----Clearing Preferences---");


}


void readPreferences();

void writeMessageID()
{
  preferences.begin("SkyCaminit", false);
  preferences.putLong("MessageID", MessageID);
  preferences.end();
#ifdef TXDEBUG
  Serial.println("----Writing MessageID Preferences---");
  Serial.print("MessageID=");
  Serial.println(MessageID);
  Serial.println("--------------------------");
#endif


}


void writeGNDR_reboots()
{
  preferences.begin("SkyCaminit", false);
  preferences.putLong("GNDR_reboots", GNDR_reboots);
  preferences.end();
#ifdef TXDEBUG
  Serial.println("----Writing  GNDR_reboots Preferences---");
  Serial.print(" GNDR_reboots=");
  Serial.println( GNDR_reboots);
  Serial.println("--------------------------");
#endif



}

void writePreferences()
{
  preferences.begin("SkyCaminit", false);

  preferences.putString("WiFi_SSID", WiFi_SSID);
  preferences.putString("WiFi_psk", WiFi_psk);



  preferences.putInt("COffsetToUTC", ClockTimeOffsetToUTC);



  preferences.putString("adminPassword", adminPassword);


  preferences.putString("MQTT_IP", MQTT_IP);
  if (MQTT_PORT == 0)
  { 
    MQTT_PORT = 1883;
  }
  preferences.putInt("MQTT_PORT", MQTT_PORT);

  preferences.putInt("time_to_sleep", time_to_sleep);
  preferences.putInt("contrast_delay", contrast_delay);


  // sensor String

  preferences.putString("Sensor_String", CurrentSensorSettings);


  preferences.putBool("blinkLight", blinkLight);


  preferences.putInt("frame_size", frame_size);

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

                     preferences.end();



#ifdef TXDEBUG
                     Serial.println("----Writing Preferences---");


                     Serial.printf("SSID="); Serial.println(WiFi_SSID);
                     Serial.printf("psk="); Serial.println(WiFi_psk);



                     Serial.print("Admin Password:");
                     Serial.println(adminPassword.substring(0, 2) + "******");



                     Serial.print("MQTT_IP=");
                     Serial.println(MQTT_IP);
                     Serial.print("MQTT_PORT=");
                     Serial.println(MQTT_PORT);
                     Serial.print("time_to_sleep=");
                     Serial.println(time_to_sleep);
                     Serial.print("contrast_delay=");
                     Serial.println(contrast_delay);
                     Serial.print("Sensor Settings=");
                     Serial.println(CurrentSensorSettings);
                     Serial.print("blinkLight=");
                     Serial.println(blinkLight);
                     Serial.print("frame_size=");
                     Serial.println(frame_size);


                     Serial.println("--------------------------");

#endif


}

void readPreferences()
{

  Serial.print("preferencesfreeentries=");
  Serial.println(preferences.freeEntries());
  preferences.begin("SkyCaminit", false);


  WiFi_SSID = preferences.getString("WiFi_SSID", "");
  WiFi_psk = preferences.getString("WiFi_psk", "");





  adminPassword = preferences.getString("adminPassword", "admin");


  MQTT_IP = preferences.getString("MQTT_IP", "");
  //MQTT_IP = "192.168.1.21";
  MQTT_PORT = preferences.getInt("MQTT_PORT", 1883);
  time_to_sleep = preferences.getInt("time_to_sleep", TIME_TO_SLEEP);
  contrast_delay = preferences.getInt("contrast_delay", DEFAULTCONTRASTDELAY);
  CurrentSensorSettings = preferences.getString("Sensor_String", DEFAULTSENSORSTRING);

  MessageID = preferences.getLong("MessageID", 0);

  GNDR_reboots = preferences.getLong("GNDR_reboots", 0);

  blinkLight = preferences.getBool("blinkLight", true);

  frame_size = preferences.getInt("frame_size", 9);  // FRAMESIZE_SVGA
  //sensor



  preferences.end();

  //writePreferences();

#ifdef TXDEBUG
  Serial.println("----Reading Preferences---");



  Serial.printf("SSID="); Serial.println(WiFi_SSID);
  Serial.printf("psk="); Serial.println(WiFi_psk);


  Serial.print("Admin Password:");
  Serial.println(adminPassword.substring(0, 2) + "******");


  Serial.print("MQTT_IP=");
  Serial.println(MQTT_IP);
  Serial.print("MQTT_PORT=");
  Serial.println(MQTT_PORT);
  Serial.print("time_to_sleep=");
  Serial.println(time_to_sleep);
  Serial.print("contrast_delay=");
  Serial.println(contrast_delay);

  Serial.print("MessageID=");
  Serial.println(MessageID);
  Serial.print("Sensor Settings=");
  Serial.println(CurrentSensorSettings);
  Serial.print("GNDR_reboots=");
  Serial.println(GNDR_reboots);

  Serial.print("blinkLight=");
  Serial.println(blinkLight);
  Serial.print("frame_size=");
  Serial.println(frame_size);
  Serial.println("--------------------------");


#endif
}
