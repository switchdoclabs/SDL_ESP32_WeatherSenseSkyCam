
#include <base64.h>

bool printReceivedData = false;

void getResponse(WiFiClient client) {
  byte buffer[8] = { NULL };
  while (client.available() > 0 || buffer[0] == NULL) {
    int len = client.available();
    //Serial.print("Len=");
    //Serial.println(len);
    if (len > 8) len = 8;
    client.read(buffer, len);
    if (printReceivedData) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
      Serial.println("");
    }
  }
}




void camera_init() {

  // QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  /*
    Serial.print("FRAMESIZE_QVGA=");
    Serial.println( FRAMESIZE_QVGA);
    Serial.print("FRAMESIZE_CIF=");
    Serial.println(FRAMESIZE_CIF);
    Serial.print("FRAMESIZE_VGA=");
    Serial.println(FRAMESIZE_VGA);
    Serial.print("FRAMESIZE_SVGA=");
    Serial.println(FRAMESIZE_SVGA);
    Serial.print("FRAMESIZE_XGA=");
    Serial.println(FRAMESIZE_XGA);
    Serial.print("FRAMESIZE_SXGA=");
    Serial.println(FRAMESIZE_SXGA);
    Serial.print("FRAMESIZE_UXGA=");
    Serial.println(FRAMESIZE_UXGA);
  */

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = 5;
  config.pin_d1       = 18;
  config.pin_d2       = 19;
  config.pin_d3       = 21;
  config.pin_d4       = 36;
  config.pin_d5       = 39;
  config.pin_d6       = 34;
  config.pin_d7       = 35;
  config.pin_xclk     = 0;
  config.pin_pclk     = 22;
  config.pin_vsync    = 25;
  config.pin_href     = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn     = 32;
  config.pin_reset    = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;


  if (frame_size == FRAMESIZE_UXGA)
  {
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    lastResolution = config.frame_size;
    config.jpeg_quality = 10; //10-63 lower number means higher quality
    config.fb_count = 2;

  }
  else
  {
    config.frame_size   = (framesize_t) frame_size; // QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    lastResolution = config.frame_size;
    config.jpeg_quality = 12
                          ;
    config.fb_count     = 1;
  }
  Serial.print("camera_init() --- config.frame_size=");
  Serial.println(config.frame_size);


  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x % x", err);


    //resetGNDRCamera();
    return;
  }

  else
  {
    Serial.println("Camera Passed Init");
  }

}





String encodeChunk(long start, long end,  camera_fb_t *fb)
{
  String returnValue;
  uint8_t binary[end - start + 1];
  int base64_length;
  Serial.print("fb->len = ");
  Serial.println(fb->len);

  int i;
  for (i = start; i < end + 1; i++)
  {
    binary[i - start] = fb->buf[i];
  }
  returnValue = base64::encode(binary, sizeof (binary));

  return returnValue;
}


int sendMQTTChunk(long MessageID, int ChunkCount, String Chunk, int TotalNumberOfChunks, int resends, long size)
{



  if (!MQTTclient.connected()) {
    MQTTreconnect(true);
  }
  MQTTclient.loop();

  String AddString;
  String SendString;

  // publish it

  AddString = "\"id\": \"";
  AddString += myID;
  AddString += "\", \"messageid\": \"";
  AddString += String(MessageID);
  AddString += "\", \"chunknumber\": \"";
  AddString += ChunkCount;
  AddString += "\", \"totalchunknumbers\": \"";
  AddString += TotalNumberOfChunks;
  AddString += "\", \"totalchunkresends\": \"";
  AddString += resends;
  AddString += "\", \"picturesize\": \"";
  AddString += size;
  AddString += "\", \"resolution\": \"";
  AddString += String(lastResolution);
  AddString += "\", \"chunk\": \"";
  AddString += Chunk;

  SendString = "{" + AddString +  "\"}"; //Send the request


  Serial.print( "Sending MQTT Chunk ");
  Serial.print(ChunkCount + 1);
  Serial.print(" of ");
  Serial.println(TotalNumberOfChunks);
  int result;
  // Once connected, publish an announcement...

  String Topic = "SKYCAM/" + myID + "/PICTURECHUNKS";

  Serial.print("Topic=");
  Serial.println(Topic);
  result = MQTTclient.publish(Topic.c_str(), SendString.c_str());
  Serial.print("MQTT publish result=");
  Serial.println(result);
  return result;
}

void take_picture(camera_fb_t * &fb) {
  fb = NULL;

  Serial.println("start of contrast delay");
  if (contrast_delay > 0) {
    delay (contrast_delay);
  }
  fb = esp_camera_fb_get();
  delay(1000);
  if (!fb) {
    Serial.println("Camera capture failed");
    lastSize = 0;


    resetGNDRCamera();

    return;
  }



  Serial.println("CLICK");
  Serial.print("Picture Size=");
  Serial.println( fb->len);
  lastSize = fb->len;
  Serial.println("after take picture");


}

void send_picture(camera_fb_t * &fb)
{
  // send out chunked messages
#define SIZEOFCHUNK 2250L

  long numberOfFullChunks;
  String chunkString;
  long lastChunkSize;

  numberOfFullChunks = fb->len / SIZEOFCHUNK;
  lastChunkSize = (fb->len) % SIZEOFCHUNK;
  Serial.print("numberOfFullChunks=");
  Serial.println(numberOfFullChunks);
  Serial.print("lastChunkSize=");
  Serial.println(lastChunkSize);


  long i;
  long TotalNumberOfChunks;

  int mqttResult;
  int resends = 0;
  for (i = 0; i < numberOfFullChunks; i++)
  {
    Serial.print("Chunk Count=");
    Serial.println(i);
    Serial.print("start of chunk=");
    Serial.print(i * SIZEOFCHUNK);
    Serial.print(" / end of chunk=");
    Serial.println((i + 1)*SIZEOFCHUNK - 1);
    chunkString = encodeChunk(i * SIZEOFCHUNK, (i + 1) * SIZEOFCHUNK - 1,  fb);

    Serial.print("ChunkStringLength=");
    Serial.println(chunkString.length());


    if (lastChunkSize > 0)
      TotalNumberOfChunks = numberOfFullChunks + 1;
    else
      TotalNumberOfChunks = numberOfFullChunks;


    mqttResult = sendMQTTChunk(MessageID, i, chunkString, TotalNumberOfChunks, resends, lastSize);
    if (mqttResult == 0)
    {
      resends++;
      mqttResult = sendMQTTChunk(MessageID, i, chunkString, TotalNumberOfChunks, resends, lastSize);  // Try one resend
      if (mqttResult == 0)
      {
        resends++;
        mqttResult = sendMQTTChunk(MessageID, i, chunkString, TotalNumberOfChunks, resends, lastSize);  // Try two resend
      }
    }
    else if (mqttResult == 0)
      break;
  }



  // final chunk
  if ((lastChunkSize > 0) && (mqttResult != 0))
  {
    // do the final chunk
    Serial.print("Chunk Count=");
    Serial.println(i);
    Serial.print("start of chunk=");
    Serial.print(i * SIZEOFCHUNK);
    Serial.print(" / end of chunk=");
    Serial.println(fb->len - 1);
    chunkString = encodeChunk(i * SIZEOFCHUNK, fb->len - 1,  fb);

    Serial.print("LastChunkStringLength=");
    Serial.println(chunkString.length());
    mqttResult = sendMQTTChunk(MessageID, i, chunkString, TotalNumberOfChunks, resends, lastSize);
    if (mqttResult == 0)
    {
      resends++;
      mqttResult = sendMQTTChunk(MessageID, i, chunkString, TotalNumberOfChunks, resends, lastSize);

    }

  }

  if (mqttResult == 0)
    Serial.println("->>>>Chunk Failure<<<<<");

  //Serial.println(chunkString);





  // Image metadata.  Yes it should be cleaned up to use printf if the function is available
  Serial.print("Size of image:");
  Serial.println(fb->len);
  Serial.print("Shape->width:");
  Serial.print(fb->width);
  Serial.print("height:");
  Serial.println(fb->height);





  esp_camera_fb_return(fb);


  Serial.println("sendMQTTStats");

  //sendMQTT(MQTTCAMERASTATUS, "");
  //sendMQTT(MQTTTEMPHUM, "");
  sendMQTT(MQTTSOLAR, "");


  MessageID++;
  writeMessageID();

}


void setSensorValues()
{
  sensor_t * s = esp_camera_sensor_get();
  int sa[22], r = 0, t = 0;
  //Serial.print("len=");
  //Serial.println(CurrentSensorSettings.length());

  for (int i = 0; i < CurrentSensorSettings.length(); i++)
  {
    //Serial.print(CurrentSensorSettings.charAt(i));
    if (CurrentSensorSettings.charAt(i) == ';')
    {
      sa[t] = CurrentSensorSettings.substring(r, i).toInt();
      //Serial.print("t=");
      //Serial.print(t);
      //Serial.print("sa[t]=");
      //Serial.println(sa[t]);
      r = (i + 1);
      t++;
    }
  }





  s->set_brightness(s, sa[0]);     // -2 to 2
  s->set_contrast(s, sa[1]);       // -2 to 2
  s->set_saturation(s, sa[2]);     // -2 to 2
  s->set_special_effect(s, sa[3]); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, sa[4]);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, sa[5]);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, sa[6]);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, sa[7]);  // 0 = disable , 1 = enable
  s->set_aec2(s, sa[8]);           // 0 = disable , 1 = enable
  s->set_ae_level(s, sa[9]);       // -2 to 2
  s->set_aec_value(s, sa[10]);    // 0 to 1200
  s->set_gain_ctrl(s, sa[11]);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, sa[12]);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)sa[13]);  // 0 to 6
  s->set_bpc(s, sa[14]);            // 0 = disable , 1 = enable
  s->set_wpc(s, sa[15]);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, sa[16]);        // 0 = disable , 1 = enable
  s->set_lenc(s, sa[17]);           // 0 = disable , 1 = enable
  s->set_hmirror(s, sa[18]);        // 0 = disable , 1 = enable
  s->set_vflip(s, sa[19]);          // 0 = disable , 1 = enable
  s->set_dcw(s, sa[20]);            // 0 = disable , 1 = enable
  s->set_colorbar(s, sa[21]);       // 0 = disable , 1 = enable

}
