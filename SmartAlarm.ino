#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "RTClib.h"
#include <JQ6500_Serial.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include "Config.h"
#include "words.h"
#include "WebUI.h"
#include "Credentials.h" 
#include "NetworkServices.h"
#include "DisplayController.h"

WiFiMulti wifiMulti;
WebServer server(80);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
RTC_DS3231 rtc;
JQ6500_Serial mp3(Serial2);
Preferences prefs; 

Alarm alarms[MAX_ALARMS];
int activeAlarmIndex = -1; 
int lastTriggeredMinute = -1; 
bool lastSwitchState = HIGH; 
int currentVolume = 10;
unsigned long lastDisplayUpdate = 0, lastWeatherUpdate = 0;
String currentWeatherTemp = "--°C", currentWind = "-- km/h";
int weatherIconCode = 69; 
bool isPauseBetweenTracks = false;
unsigned long trackPauseStartTime = 0;
int selectedWordIndex = 0;
bool isLightOn = false; 
bool isManualBrightness = false;
int manualContrast = 150;
bool isTrainsEnabled = true;
bool forceShowTrains = false;
String myLat = "50.10";
String myLon = "8.76";

bool showTrainScheduleMode = false;
unsigned long alarmStopTime = 0;
unsigned long lastTrainFetchTime = 0;
unsigned long lastTrainPageSwitch = 0;
int trainPage = 0;

bool isCustomTextMode = false;
String customScreenText = "";

TrainInfo fetchedTrains[10];
int fetchedTrainsCount = 0;

void initAlarms() {
  for(int i=0; i<MAX_ALARMS; i++) {
    alarms[i] = {false, 0, 0, 1, true, {true,true,true,true,true,false,false}};
  }
  alarms[0] = {true, 7, 0, 1, true, {true,true,true,true,true,false,false}}; 
}

void loadSettingsFromMemory() {
  prefs.begin("alarm_app", true); 
  if (prefs.getBytesLength("data") == sizeof(alarms)) {
    prefs.getBytes("data", alarms, sizeof(alarms));
  } else {
    initAlarms(); 
  }
  isManualBrightness = prefs.getBool("manBright", false);
  manualContrast = prefs.getInt("manContr", 150);
  isTrainsEnabled = prefs.getBool("trainsEn", true); 
  prefs.end();
}

void saveAlarmsToMemory() {
  prefs.begin("alarm_app", false); 
  prefs.putBytes("data", alarms, sizeof(alarms));
  prefs.end();
}

void startAlarm(int index) {
  activeAlarmIndex = index;
  currentVolume = 9; 
  isPauseBetweenTracks = false;
  isCustomTextMode = false;
  forceShowTrains = false;

  if (alarms[index].sunrise) {
    digitalWrite(RELAY_PIN, LOW); 
    isLightOn = true;
  }

  mp3.reset(); 
  delay(200);
  mp3.setVolume(currentVolume); 
  delay(100);
  mp3.playFileByIndexNumber(alarms[index].melody);
  delay(100);
  mp3.playFileByIndexNumber(alarms[index].melody);
  
  selectedWordIndex = random(0, totalWords);
}

void stopAlarm() { 
  activeAlarmIndex = -1; 
  digitalWrite(RELAY_PIN, HIGH); 
  isLightOn = false;
  mp3.setVolume(0); 
  delay(50);
  mp3.pause(); 

  if (isTrainsEnabled) {
    showTrainScheduleMode = true;
    alarmStopTime = millis();
    lastTrainFetchTime = millis();
    
    char alarmTimeBuf[6];
    sprintf(alarmTimeBuf, "%02d:%02d", alarms[activeAlarmIndex].hour, alarms[activeAlarmIndex].minute);
    NetworkService::fetchTrainSchedule(String(alarmTimeBuf));
  }
}

void setupServers() {
  server.on("/", HTTP_GET, [](){ server.send(200, "text/html", index_html); });
  
  server.on("/api/status", HTTP_GET, [](){
    JsonDocument doc;
    DateTime now = rtc.now();
    char timeBuf[10]; sprintf(timeBuf, "%02d:%02d", now.hour(), now.minute());
    doc["time"] = timeBuf;
    doc["light"] = isLightOn; 
    doc["isManualBright"] = isManualBrightness; 
    doc["bright"] = manualContrast;
    doc["trainsEnabled"] = isTrainsEnabled;
    JsonArray arr = doc["alarms"].to<JsonArray>();
    for(int i=0; i<MAX_ALARMS; i++) {
      if(alarms[i].hour == 0 && alarms[i].minute == 0 && !alarms[i].active && !alarms[i].sunrise) continue; 
      JsonObject obj = arr.add<JsonObject>();
      obj["active"] = alarms[i].active; obj["h"] = alarms[i].hour;
      obj["m"] = alarms[i].minute; obj["melody"] = alarms[i].melody;
      obj["sunrise"] = alarms[i].sunrise; 
      JsonArray daysArr = obj["days"].to<JsonArray>();
      for(int j=0; j<7; j++) daysArr.add(alarms[i].days[j]);
    }
    String r; serializeJson(doc, r); server.send(200, "application/json", r);
  });

  server.on("/api/light", HTTP_POST, [](){
    isLightOn = !isLightOn; 
    digitalWrite(RELAY_PIN, isLightOn ? LOW : HIGH); 
    server.send(200, "application/json", isLightOn ? "{\"state\":true}" : "{\"state\":false}");
  });

  server.on("/api/trains-config", HTTP_POST, [](){
    JsonDocument doc; deserializeJson(doc, server.arg("plain"));
    isTrainsEnabled = doc["enabled"];
    if (!isTrainsEnabled) forceShowTrains = false;
    prefs.begin("alarm_app", false); prefs.putBool("trainsEn", isTrainsEnabled); prefs.end();
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/trains-show", HTTP_POST, [](){
    JsonDocument doc; deserializeJson(doc, server.arg("plain"));
    bool show = doc["show"];
    if (isTrainsEnabled) {
      forceShowTrains = show;
      if (show) NetworkService::fetchTrainSchedule("");
    }
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/trains-query", HTTP_POST, [](){
    JsonDocument doc; deserializeJson(doc, server.arg("plain"));
    String t = doc["time"].as<String>();
    if (isTrainsEnabled) {
      forceShowTrains = true;
      showTrainScheduleMode = false;
      NetworkService::fetchTrainSchedule(t); 
    }
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/brightness", HTTP_POST, [](){
    JsonDocument doc; deserializeJson(doc, server.arg("plain"));
    isManualBrightness = !doc["auto"];
    manualContrast = doc["val"];
    prefs.begin("alarm_app", false); prefs.putBool("manBright", isManualBrightness); prefs.putInt("manContr", manualContrast); prefs.end();
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/message", HTTP_POST, [](){
    JsonDocument doc; deserializeJson(doc, server.arg("plain"));
    isCustomTextMode = doc["active"]; customScreenText = doc["text"].as<String>();
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/alarms", HTTP_POST, [](){
    JsonDocument doc; deserializeJson(doc, server.arg("plain"));
    JsonArray arr = doc.as<JsonArray>();
    for(int i=0; i<MAX_ALARMS; i++) alarms[i].active = false; 
    for(int i=0; i<arr.size() && i<MAX_ALARMS; i++) {
      alarms[i].active = arr[i]["active"]; alarms[i].hour = arr[i]["h"];
      alarms[i].minute = arr[i]["m"]; alarms[i].melody = arr[i]["melody"];
      alarms[i].sunrise = arr[i]["sunrise"] | true; 
      for(int j=0; j<7; j++) alarms[i].days[j] = arr[i]["days"][j];
    }
    saveAlarmsToMemory(); server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/time", HTTP_POST, [](){
    JsonDocument doc; deserializeJson(doc, server.arg("plain"));
    rtc.adjust(DateTime(doc["y"], doc["mo"], doc["d"], doc["h"], doc["m"], doc["s"]));
    server.send(200, "application/json", "{\"ok\":true}");
  });
  
  server.begin();
}

void setup() {
  Serial.begin(115200); Serial2.begin(9600, SERIAL_8N1, 16, 17);
  pinMode(BUTTON_PIN, INPUT_PULLUP); lastSwitchState = digitalRead(BUTTON_PIN);
  
  digitalWrite(RELAY_PIN, HIGH); pinMode(RELAY_PIN, OUTPUT);    

  Wire.begin(21, 22); u8g2.begin(); u8g2.enableUTF8Print();
  rtc.begin(); mp3.reset(); 
  loadSettingsFromMemory();
  
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
  
  while (wifiMulti.run() != WL_CONNECTED) { 
    u8g2.clearBuffer(); u8g2.setFont(u8g2_font_unifont_t_cyrillic);
    u8g2.setCursor(10, 35); u8g2.print("Verbindung..."); u8g2.sendBuffer(); delay(500);
  }

  MDNS.begin("alarm");
  ArduinoOTA.setHostname("SmartAlarm"); ArduinoOTA.begin();
  
  setupServers();
  NetworkService::fetchLocation();
  NetworkService::fetchWeather();
}

void loop() {
  ArduinoOTA.handle(); 
  server.handleClient();
  DateTime now = rtc.now();

  if (WiFi.status() != WL_CONNECTED && millis() % 10000 == 0) wifiMulti.run();

  if (millis() - lastWeatherUpdate > 180000 || (currentWeatherTemp == "--°C" && millis() - lastWeatherUpdate > 10000)) {
    if (WiFi.status() == WL_CONNECTED) {
      if (myLat == "" || myLon == "") NetworkService::fetchLocation();
      NetworkService::fetchWeather();
      lastWeatherUpdate = millis();
    }
  }

  if (isTrainsEnabled && (showTrainScheduleMode || forceShowTrains)) {
    if (showTrainScheduleMode && millis() - alarmStopTime > SCHEDULE_DURATION && !forceShowTrains) {
      showTrainScheduleMode = false;
    } else {
      if (millis() - lastTrainFetchTime > TRAIN_FETCH_INTERVAL) {
        NetworkService::fetchTrainSchedule("");
        lastTrainFetchTime = millis();
      }
    }
  } else if (!isTrainsEnabled) {
    showTrainScheduleMode = false; forceShowTrains = false;
  }

  bool sw = digitalRead(BUTTON_PIN);
  if (sw != lastSwitchState) { 
    delay(50); 
    if (activeAlarmIndex != -1) {
      stopAlarm(); 
    } else {
      isLightOn = !isLightOn;
      digitalWrite(RELAY_PIN, isLightOn ? LOW : HIGH);
    }
    lastSwitchState = sw; 
  }

  if (millis() - lastDisplayUpdate > 100) { 
    DisplayController::updateBrightness(now);
    DisplayController::update(now);
    lastDisplayUpdate = millis();
    
    int today = now.dayOfTheWeek() - 1;
    if (today < 0) today = 6; 

    if (activeAlarmIndex == -1 && lastTriggeredMinute != now.minute()) {
      for(int i=0; i<MAX_ALARMS; i++) {
        if(alarms[i].active && alarms[i].days[today] && alarms[i].hour == now.hour() && alarms[i].minute == now.minute()) {
          startAlarm(i);
          lastTriggeredMinute = now.minute(); 
          break; 
        }
      }
    }
  }

  if (activeAlarmIndex != -1) {
    static unsigned long lastWordUpdate = 0;
    if (millis() - lastWordUpdate > 4000) {
        selectedWordIndex = random(0, totalWords); lastWordUpdate = millis();
    }
    
    if (mp3.getStatus() != MP3_STATUS_PLAYING && !isPauseBetweenTracks) {
        if (currentVolume == 9) { currentVolume = 18; mp3.setVolume(currentVolume); delay(50); }
        else if (currentVolume == 18) { currentVolume = 30; mp3.setVolume(currentVolume); delay(50); }
        mp3.playFileByIndexNumber(alarms[activeAlarmIndex].melody); delay(100); 
    } 
    
    if (mp3.getStatus() != MP3_STATUS_PLAYING && isPauseBetweenTracks) {
        if (millis() - trackPauseStartTime > PAUSE_DURATION) {
            mp3.playFileByIndexNumber(alarms[activeAlarmIndex].melody);
            isPauseBetweenTracks = false;
        }
    }
  }
}