#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Config.h"

extern String myLat;
extern String myLon;
extern String currentWeatherTemp;
extern String currentWind;
extern int weatherIconCode;
extern bool isTrainsEnabled;
extern TrainInfo fetchedTrains[10];
extern int fetchedTrainsCount;

class NetworkService {
public:
  static void fetchLocation() {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin("http://ip-api.com/json/");
      if (http.GET() == 200) {
        JsonDocument doc; 
        deserializeJson(doc, http.getString());
        if (doc["status"] == "success") {
          myLat = doc["lat"].as<String>();
          myLon = doc["lon"].as<String>();
        }
      }
      http.end();
    }
  }

  static void fetchWeather() {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = "http://api.open-meteo.com/v1/forecast?latitude=" + myLat + "&longitude=" + myLon + "&current_weather=true";
      http.begin(url);
      if (http.GET() == 200) {
        JsonDocument doc; deserializeJson(doc, http.getString());
        currentWeatherTemp = String((float)doc["current_weather"]["temperature"], 1) + "°C";
        currentWind = String((float)doc["current_weather"]["windspeed"], 1) + " km/h";
        int code = doc["current_weather"]["weathercode"];
        if (code == 0) weatherIconCode = 69; 
        else if (code <= 3) weatherIconCode = 64; 
        else weatherIconCode = 67;
      }
      http.end();
    }
  }

  static void fetchTrainSchedule(String timeStr = "") {
    if (!isTrainsEnabled) return; 
    
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = API_SERVER_URL;
      if (timeStr != "") url += "?time=" + timeStr;
      
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          JsonArray arr = doc["trains"].as<JsonArray>();
          fetchedTrainsCount = 0;
          for (JsonVariant v : arr) {
            if (fetchedTrainsCount < 10) {
              fetchedTrains[fetchedTrainsCount].text = v.as<String>();
              fetchedTrainsCount++;
            }
          }
        }
      }
      http.end();
    }
  }
};