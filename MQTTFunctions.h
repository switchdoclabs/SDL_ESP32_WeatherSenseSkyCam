


void camera_init() ;  // forward declaration



int sendMQTT(int messageType, String argument)
{

  String AddString;

  String SendString;

  if (!MQTTclient.connected()) {
    MQTTreconnect(true);
  }
  MQTTclient.loop();





  switch (messageType)
  {


    case MQTTTESTMESSAGE:
      {


        AddString = "\"id\": \"";
        AddString += myID;
        AddString += "\", \"messageid\": \"";
        AddString += String(MessageID);
        AddString += "\", \"messagetype\": \"";
        AddString += messageType;

        SendString = "{" + AddString +  "\"}"; //Send the request

        break;
      }

    case MQTTDEBUG:
      {
        AddString = "\"id\": \"";
        AddString += myID;
        AddString += "\", \"messageid\": \"";
        AddString += String(MessageID);
        AddString += "\", \"messagetype\": \"";
        AddString += messageType;

        AddString += "\", \"value\": \"";
        AddString += argument;

        SendString = "{" + AddString +  "\"}"; //Send the request
        break;


      }

    case MQTTLASTPICMESSAGEID:
      {
        AddString = "\"id\": \"";
        AddString += myID;
        AddString += "\", \"messageid\": \"";
        AddString += String(MessageID);
        AddString += "\", \"messagetype\": \"";
        AddString += messageType;

        SendString = "{" + AddString +  "\"}"; //Send the request
        break;
      }

    case MQTTCAMERASTATUS:
      {
        AddString = "\"id\": \"";
        AddString += myID;
        AddString += "\", \"messageid\": \"";
        AddString += String(MessageID);
        AddString += "\", \"messagetype\": \"";
        AddString += messageType;

        AddString += "\", \"size\": \"";
        AddString += String(lastSize);
        AddString += "\", \"resolution\": \"";
        AddString += String(lastResolution);


        SendString = "{" + AddString +  "\"}"; //Send the request
        break;


      }
    case MQTTTEMPHUM:
      {


        AddString = "\"id\": \"";
        AddString += myID;
        AddString += "\", \"messageid\": \"";
        AddString += String(MessageID);
        AddString += "\", \"messagetype\": \"";
        AddString += messageType;
        AddString += "\", \"softwareversion\": \"";
        AddString += String(SOFTWAREVERSION);
        AddString += "\", \"currentrssi\": \"";
        AddString += String(currentRSSI);
        AddString += "\", \"devicepresent\": \"";
        AddString += String(HDC1080_Present);
        AddString += "\", \"temperature\": \"";


        AddString += String(InsideTemperature);



        AddString += "\", \"humidity\": \"";

        AddString += String(InsideHumidity);

        SendString = "{" + AddString +  "}"; //Send the request

        break;
      }
    case MQTTSOLAR:

      {

        AddString = "\"id\": \"";
        AddString += myID;
        AddString += "\", \"messageid\": \"";
        AddString += String(MessageID);
        AddString += "\", \"messagetype\": \"";
        AddString += messageType;

        AddString += "\", \"softwareversion\": \"";
        AddString += String(SOFTWAREVERSION);

        AddString += "\", \"sunairplusdevicepresent\": \"";
        AddString += String(SunAirPlus_Present);

        AddString += "\", \"hdc1080devicepresent\": \"";
        AddString += String(HDC1080_Present);

        AddString += "\", \"internaltemperature\": \"";
        AddString += String(InsideTemperature);

        AddString += "\", \"internalhumidity\": \"";
        AddString += String(InsideHumidity);


        AddString += "\", \"currentrssi\": \"";
        AddString += String(currentRSSI);


        AddString += "\", \"batteryvoltage\": \"";
        AddString += String(BatteryVoltage);


        AddString += "\", \"batterycurrent\": \"";
        AddString += String(BatteryCurrent);


        AddString += "\", \"loadvoltage\": \"";
        AddString += String(LoadVoltage);


        AddString += "\", \"loadcurrent\": \"";
        AddString += String(LoadCurrent);


        AddString += "\", \"solarpanelvoltage\": \"";
        AddString += String(SolarPanelVoltage);


        AddString += "\", \"solarpanelcurrent\": \"";
        AddString += String(SolarPanelCurrent);

        AddString += "\", \"gndrreboots\": \"";
        AddString += String(GNDR_reboots);

        AddString += "\"";


        SendString = "{" + AddString +  "}"; //Send the request


        break;
      }
    default:
      break;


  }


  // publish it

  Serial.println( "Sending MQTT Packet");
  Serial.println(SendString);
  int result;
  // Once connected, publish an announcement...

  String Topic = "SKYCAM/" + myID + "/INFO";
  if (messageType == MQTTLASTPICMESSAGEID)
  {
    Topic = "SKYCAM/" + myID + "/PICTUREID";
  }
  Serial.print("Topic=");
  Serial.println(Topic);
  result = MQTTclient.publish(Topic.c_str(), SendString.c_str());
  Serial.print("MQTT publish result=");
  Serial.println(result);
  return result;
}




void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Arrived Topic:");
  Serial.println(topic);
  Serial.print("Message arrived ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  char buffer[400];
  int i;
  for (i = 0; i < length; i++)
  {
    buffer[i] = payload[i];
  }
  buffer[i] = 0;

  StaticJsonDocument<400> myJSON;
  // serialize to JSON then interpret
  DeserializationError error = deserializeJson(myJSON, buffer);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }


  int messagetype = myJSON["messagetype"];
  Serial.print("messagetype=");
  Serial.println(messagetype);



  switch (messagetype)
  {

    case MQTTUPDATEPARAM:

      {

        String sendSensor =  myJSON["sensorparams"];

        int count = 0;
        for (int i = 0; i < sendSensor.length(); i++) {

          if (sendSensor.charAt(i) == ';')
          {


            count++;
          }
        }
        if (count != 22)
        {
          Serial.print("Bad Sensor String from Pi.  Count of semicolons=");
          Serial.println(count);
        }

        CurrentSensorSettings =  sendSensor;
        writePreferences();





        break;
      }

    case MQTTBLINKXTIMES:
      {
        int length = myJSON["length"];
        int count = myJSON["count"];
        blinkLED(count, length);

      }
      break;
    case MQTTCYCLECHANGE:
      {
        int sleeptime = myJSON["timetosleep"];

        if (sleeptime > -1)
        {
          time_to_sleep = sleeptime;
          writePreferences();

          // reset timing on watchdog timer
          //wdt_timeout = time_to_sleep * 3 + 3 * 10;
          //esp_task_wdt_init(wdt_timeout, true); //enable panic so ESP32 restarts
          //esp_task_wdt_add(NULL); //add current thread to WDT watch
        }

      }
      break;

    case MQTTRESOLUTION:
      {
        int framesize = myJSON["framesize"];

        if ((framesize > 4) && (framesize < 14))
        {
          frame_size = framesize;
          writePreferences();

          // reboot to change framesize
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          ESP.restart();
        }

      }
      break;

    case MQTTSTARTDELAY:
      {
        int temp = myJSON["contrastdelay"];

        if (temp > -1)
        {
          contrast_delay = temp;
          writePreferences();
          delay(1000);
        }

      }
      break;

    case MQTTREBOOT:
      {
        ESP.restart();
      }
      break;

    case  MQTTTURNOFFBLINK:
      {
        blinkLight = false;
      }
      break;

    case MQTTERASEMEMORY:
      {

        resetPreferences();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart();
      }
      break;

      case MQTTSETMQTTIPPORT:
      {

       MQTT_IP = myJSON['mqttip'].as<String>();

       MQTT_PORT = int(myJSON["mqttport"]);
      writePreferences();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart();

      }

    case  MQTTSETTODEFAULTS :
      {
        // does not reset ssid, password, mqtt ip or mqtt port

        blinkLight = true;
        time_to_sleep = TIME_TO_SLEEP;
        contrast_delay = DEFAULTCONTRASTDELAY;
        CurrentSensorSettings = DEFAULTSENSORSTRING;
        writePreferences();

      }
      break;


    default:

      Serial.print("unsupported incoming MQTT Message:");

      Serial.print(buffer);

      Serial.println();
      break;
  }
  return;

}
