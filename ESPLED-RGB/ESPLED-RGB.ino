#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "esp_wpa2.h"
#include <FastLED.h>

#include "ESPAsyncWebServer.h"

#include "config.c"

#if USE_DISPLAY
  unsigned long previousUpdateTime;
#endif

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define VERSION "1.0.0"

bool power_state = false;
byte brightness  = STARTUP_BRIGHTNESS;

CRGB leds[NUM_LEDS];

const size_t capacity = JSON_ARRAY_SIZE(288) + 288*JSON_OBJECT_SIZE(2) + 6080; // https://arduinojson.org/v6/assistant/

//WiFiClientSecure client;
//WiFiServer wifiServer(80);

AsyncWebServer server(80);
AsyncWebSocket ws("/websocket");

void setup() {
  #if DISABLE_BROWNOUT_SENSOR
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  #endif
  
  Serial.begin(115200);
  
  delay(1000);
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( STARTUP_BRIGHTNESS );
  FastLED.delay(1000 / UPDATE_FREQUENCY); 
  
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  
  FastLED.show();

  #if USE_DISPLAY
    initializeDisplay();
  #endif

  WiFi.disconnect(true);

  #if WIFI_WPA2_ENT
    WiFi.mode(WIFI_STA);
    
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)); //provide identity
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)); //provide username --> identity and username is same
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD)); //provide password
    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); //set config settings to default
    esp_wifi_sta_wpa2_ent_enable(&config); //set config settings to enable function

    WiFi.begin(WIFI_SSID);
  #else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  #endif
  
  
  Serial.print("Connecting to WiFi...");

  int  light_position = 0;
  bool going_right    = true;
  
  while (WiFi.status() != WL_CONNECTED) {
    if (going_right) {
      if (light_position + 1 == NUM_LEDS) {
        going_right = false;
      } else {
  
        if (light_position > 0) {
          leds[light_position - 1] = CRGB::Black;
        }
       
        leds[light_position] = CRGB::Blue;
        light_position += 1;
      }
    } else {

      if (light_position == 0) {
        going_right = true;
      } else {
  
        if (light_position + 1 < NUM_LEDS) {
          leds[light_position + 1] = CRGB::Black;
        }
       
        leds[light_position] = CRGB::Blue;
        light_position -= 1;
      }
    }

    FastLED.show();
    delay(1);
  }
  
  Serial.println();
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());

  #if OTA_ENABLED
    setupOTA();
  #endif

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  
  FastLED.show();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();
}

void loop() {
  #if OTA_ENABLED
    handleOTA();
  #endif

  #if USE_DISPLAY
    if (previousUpdateTime + (1000 / UPDATE_FREQUENCY) < millis()) {
      previousUpdateTime = millis();
      updateFrame();
    }
    
  #endif
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){

  if(type == WS_EVT_CONNECT){

    Serial.println("Websocket client connection received");

    StaticJsonDocument<200> connectResponse;
    connectResponse["data_type"]   = "STATUS";
    connectResponse["device_name"] = DEVICE_NAME;
    connectResponse["version"]     = VERSION;
    connectResponse["led_count"]   = NUM_LEDS;
    connectResponse["power_state"] = power_state;
    connectResponse["brightness"]  = map(FastLED.getBrightness(), 0, 255, 0, 100);
    
    char connectResponseData[200];
    serializeJson(connectResponse, connectResponseData, 200);
    client->text(connectResponseData);

  } else if(type == WS_EVT_DISCONNECT){
    
    Serial.println("Client disconnected");
    
  } else if(type == WS_EVT_DATA){

    Serial.println("Data received: ");

    for(int i=0; i < len; i++) {
      Serial.print(char(data[i]));
      Serial.print("|");
    }
    
    DynamicJsonDocument doc(capacity);
    //StaticJsonDocument<200> responseJson;
    DeserializationError error = deserializeJson(doc, data);
  
    if (error) {
      Serial.println("Err");
      reportError(client, "", "Unable to parse JSON");
    } else {

      if (doc["data_type"] == "setBrightness") {
        if (!power_state) {
          reportError(client, "setBrightness", "Power is turned off");
          
          return;
        }
        
        brightness = map(doc["data"], 0, 100, 0, 255);
        FastLED.setBrightness(brightness);
        FastLED.show();

        reportSuccess(client, "setBrightness");

      } else if (doc["data_type"] == "setPower") {
        
        power_state = doc["data"];
        
        if (power_state) {
          FastLED.setBrightness(brightness);
        } else {
          FastLED.setBrightness(0);
        }

        FastLED.show();
        
        reportSuccess(client, "setPower");
        
      } else if (doc["data_type"] == "setAllRGB") {
        if (!power_state) {
          reportError(client, "setAllRGB", "Power is turned off");
          
          return;
        }
        
        long color = strtol(doc["data"]["hexColor"], NULL, 16);
        
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = color;
        }

        FastLED.show();

        reportSuccess(client, "setAllRGB");
        
      } else {
        reportError(client, "", "Unknown data_type");
      }
    }

    Serial.println();
  }
}

void reportError(AsyncWebSocketClient * client, String action, String error) {
  StaticJsonDocument<200> responseJson;

  responseJson["data_type"] = "ERROR";
  responseJson["success"] = false;
  responseJson["action"] = action;
  responseJson["error"] = error;
    
  char result[200];
  serializeJson(responseJson, result, 200);
  client->text(result);
}

void reportSuccess(AsyncWebSocketClient * client, String action) {
  StaticJsonDocument<200> responseJson;

  responseJson["data_type"] = "RESPONSE";
  responseJson["success"]   = true;
  responseJson["action"]    = action;
  
  char result[200];
  serializeJson(responseJson, result, 200);
  client->text(result);
}

void broadcastEvent(String eventType, String eventValue, bool closeConnection) {
  char result[200];
  StaticJsonDocument<200> responseJson;
  
 
  if (eventValue == "") {
    responseJson["event_value"] = nullptr;
  } else {
    responseJson["event_value"] = eventValue;
  }
  
  responseJson["event_type"]         = eventType;
  responseJson["data_type"]          = "BROADCAST";
  responseJson["connection_closing"] = closeConnection;
  
  serializeJson(responseJson, result, 200);
  
  ws.textAll(result);

  if (closeConnection) {
    ws.enable(false);
    ws.closeAll();
  }
}
