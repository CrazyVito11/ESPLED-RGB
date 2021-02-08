#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#if USE_DISPLAY
  unsigned long updateProgressLastFrameUpdate;
#endif

void setupOTA() {
  
   ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      #if USE_DISPLAY
        firmwareUpdateStart();
      #endif
      
      broadcastEvent("SYSTEM_UPDATE", "", true);
      
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      
      #if USE_DISPLAY
        if (updateProgressLastFrameUpdate + 1000 < millis()) {
          updateProgressLastFrameUpdate = millis();
          
          firmwareUpdateOnProgress((progress / (total / 100)));
        }
      #endif
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setHostname(DEVICE_NAME);
  ArduinoOTA.begin();
  
  Serial.println("OTA enabled @ " + String(OTA_PORT));
}

void handleOTA() {
  ArduinoOTA.handle();
}
