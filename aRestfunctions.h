//
//
//
// aRest Command functions
//
//

// parsing function
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1
  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// Custom function accessible by the API

// general commands

int checkForID(String command) {
  Serial.println("---------->REST: checkForID");
  Serial.print("Command =");
  Serial.println(command);

  MQTT_IP = getValue(command, ',', 0);
  MQTT_PORT = (getValue(command, ',', 1)).toInt();

  writePreferences();

  RESTreturnString = myID + ",";

  RESTreturnString += SKYCAMSOFTWAREVERSION;


  MQTTclient.setServer(MQTT_IP.c_str(), MQTT_PORT);
  MQTTclient.setCallback(MQTTcallback);
  //blinkIPAddress();
  MQTTreconnect(false);
  return 0;

}

int restartMQTT(String command) {
  Serial.println("---------->REST: restartMQTT");
  Serial.print("Command =");
  Serial.println(command);
  String password;
  password = getValue(command, ',', 0);
  if (password == adminPassword)
  {


    RESTreturnString = "";

    MQTTclient.setServer(MQTT_IP.c_str(), MQTT_PORT);
    MQTTclient.setCallback(MQTTcallback);
    //blinkIPAddress();
    MQTTreconnect(false);


  }
  else
    return 1;
  return 0;

}
