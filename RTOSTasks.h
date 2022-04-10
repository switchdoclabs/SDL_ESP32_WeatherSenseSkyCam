// RTOS Tasks for various modes and periphials


void taskRESTCommand( void * parameter)
{
  // Enter RTOS Task Loop

  Serial.println("------------Starting REST-----------");
  for (;;)  {

    if (uxSemaphoreGetCount( xSemaphoreRESTCommand ) > 0)
    {

      // Handle REST calls
      WiFiClient client = MyServer.available();


      int timeout;
      timeout = 0;
      if (client)
      {
        /*
          Serial.print("SA client=");
          Serial.println(client);

          Serial.print("SA client.available()=");
          Serial.println(client.available());

          // Thank you to MAKA69 for this suggestion
          while (!client.available()) {
          Serial.print(".");
          vTaskDelay(10 / portTICK_PERIOD_MS);
          timeout++;
          Serial.print("IL client.available()=");
          Serial.println(client.available());
          if (timeout > 1000) {
            Serial.println("INFINITE LOOP BREAK!");
            break;
          }
          }

          Serial.print("AI client.available()=");
          Serial.println(client.available());
          */
        
        if (client.available())
        {



          rest.handle(client);

        }
      }
      client.stop();

      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    else
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


  }

}
