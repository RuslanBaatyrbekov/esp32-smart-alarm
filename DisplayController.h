#pragma once
#include <U8g2lib.h>
#include <Wire.h>
#include "RTClib.h"
#include "Config.h"
#include "words.h"

// Сообщаем компилятору, что переменные лежат в основном файле
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern Alarm alarms[MAX_ALARMS];
extern int activeAlarmIndex;
extern bool isManualBrightness;
extern int manualContrast;
extern bool isTrainsEnabled;
extern bool forceShowTrains;
extern bool showTrainScheduleMode;
extern bool isCustomTextMode;
extern String customScreenText;
extern String currentWeatherTemp;
extern int weatherIconCode;
extern int selectedWordIndex;
extern TrainInfo fetchedTrains[10];
extern int fetchedTrainsCount;
extern int trainPage;
extern unsigned long lastTrainPageSwitch;

class DisplayController {
private:
  static void drawWifiIcon(int x, int y) {
    if (WiFi.status() != WL_CONNECTED) return;
    long rssi = WiFi.RSSI();
    u8g2.drawBox(x, y - 2, 2, 2); 
    if (rssi > -80) u8g2.drawBox(x + 3, y - 4, 2, 4); 
    if (rssi > -70) u8g2.drawBox(x + 6, y - 6, 2, 6); 
    if (rssi > -60) u8g2.drawBox(x + 9, y - 8, 2, 8); 
  }

  static String getNextAlarmText() {
    for(int i=0; i<MAX_ALARMS; i++) {
      if(alarms[i].active) {
        char buf[10];
        sprintf(buf, "AL:%02d:%02d", alarms[i].hour, alarms[i].minute);
        return String(buf);
      }
    }
    return "AL:OFF";
  }

public:
  static void updateBrightness(DateTime now) {
    int contrastVal = 0x9F; 
    int vcomhVal = 0x20;

    if (activeAlarmIndex != -1) {
      contrastVal = 0xFF; 
      vcomhVal = 0x30;    
    } else if (isManualBrightness) {
      contrastVal = manualContrast;
      vcomhVal = (manualContrast < 50) ? 0x00 : 0x20; 
    } else if (now.hour() >= 22 || now.hour() < 6) {
      contrastVal = 0x1A; 
      vcomhVal = 0x00;    
    } else if (now.hour() >= 7 && now.hour() < 21) {
      contrastVal = 0x9F; 
      vcomhVal = 0x20;
    } else if (now.hour() == 6) {
      contrastVal = map(now.minute(), 0, 59, 0x1A, 0x9F);
      vcomhVal = 0x20;
    } else if (now.hour() == 21) {
      contrastVal = map(now.minute(), 0, 59, 0x9F, 0x1A);
      vcomhVal = (now.minute() > 30) ? 0x00 : 0x20;
    }

    Wire.beginTransmission(0x3C); 
    Wire.write(0x00); 
    Wire.write(0x81); Wire.write(contrastVal); 
    Wire.write(0xD9); Wire.write(0x22); 
    Wire.write(0xDB); Wire.write(vcomhVal); 
    Wire.endTransmission();
  }

  static void update(DateTime now) {
    u8g2.clearBuffer();
    
    if (activeAlarmIndex != -1) {
      u8g2.setFont(u8g2_font_helvB08_tf);
      u8g2.setCursor(0, 10);
      u8g2.print("["); u8g2.print(dailyWords[selectedWordIndex].level); u8g2.print("]");

      int pulse = 1 + (int)(2.0 * sin(millis() / 200.0));
      u8g2.drawCircle(64, 23, 6 + pulse); 
      for(int i=0; i<8; i++) { 
        float angle = i * 45 * 0.0174;
        int x1 = 64 + cos(angle) * (8 + pulse);
        int y1 = 23 + sin(angle) * (8 + pulse);
        int x2 = 64 + cos(angle) * (13 + pulse);
        int y2 = 23 + sin(angle) * (13 + pulse);
        u8g2.drawLine(x1, y1, x2, y2);
      }
      
      u8g2.setFont(u8g2_font_unifont_t_latin);
      u8g2.setCursor(0, 47); u8g2.print(dailyWords[selectedWordIndex].word);
      
      u8g2.setFont(u8g2_font_unifont_t_cyrillic);
      u8g2.setCursor(0, 64); u8g2.print(dailyWords[selectedWordIndex].translation);

    } else if (isTrainsEnabled && (showTrainScheduleMode || forceShowTrains)) {
      u8g2.setFont(u8g2_font_5x7_tf);
      char timeBuf[10];
      sprintf(timeBuf, "%02d:%02d", now.hour(), now.minute());
      u8g2.setCursor(0, 8); u8g2.print(timeBuf);
      u8g2.setCursor(45, 8); u8g2.print("[ПОЕЗДА]");
      drawWifiIcon(120, 9);
      u8g2.drawLine(0, 10, 128, 10);

      if (millis() - lastTrainPageSwitch > 10000) {
        if (fetchedTrainsCount > 3) {
          trainPage = (trainPage + 1) % ((fetchedTrainsCount + 2) / 3);
        } else {
          trainPage = 0;
        }
        lastTrainPageSwitch = millis();
      }

      u8g2.setFont(u8g2_font_helvR08_tf);
      if (fetchedTrainsCount == 0) {
        u8g2.setCursor(10, 35);
        u8g2.print("Загрузка расписания...");
      } else {
        int startIdx = trainPage * 3;
        int yPos = 22;
        for (int i = startIdx; i < startIdx + 3 && i < fetchedTrainsCount; i++) {
          u8g2.setCursor(0, yPos);
          u8g2.print(fetchedTrains[i].text);
          yPos += 14;
        }
      }

    } else if (isCustomTextMode) {
      u8g2.setFont(u8g2_font_5x7_tf);
      char timeBuf[10];
      sprintf(timeBuf, "%02d:%02d", now.hour(), now.minute());
      u8g2.setCursor(0, 8); u8g2.print(timeBuf); 
      u8g2.setCursor(45, 8); u8g2.print("[МЕМО]"); 
      drawWifiIcon(120, 9);
      u8g2.drawLine(0, 10, 128, 10);

      u8g2.setFont(u8g2_font_unifont_t_cyrillic);
      
      int startY = 28;
      int lineHeight = 16;
      int maxCharsPerLine = 11;
      
      int len = customScreenText.length();
      int charCount = 0;
      int lineIndex = 0;
      String currentLine = "";
      
      int i = 0;
      while (i < len && lineIndex < 3) {
        char c = customScreenText[i];
        int charLen = 1;
        
        if ((c & 0xE0) == 0xC0) charLen = 2;
        else if ((c & 0xF0) == 0xE0) charLen = 3;
        
        String letter = customScreenText.substring(i, i + charLen);
        i += charLen;
        
        currentLine += letter;
        charCount++;
        
        if (charCount >= maxCharsPerLine || i >= len) {
          u8g2.setCursor(0, startY + (lineIndex * lineHeight));
          u8g2.print(currentLine);
          currentLine = "";
          charCount = 0;
          lineIndex++;
        }
      }

    } else {
      u8g2.setFont(u8g2_font_5x7_tf);
      u8g2.setCursor(0, 8); u8g2.print(WiFi.localIP().toString()); 
      u8g2.setCursor(75, 8); u8g2.print(getNextAlarmText());
      drawWifiIcon(120, 9); 
      u8g2.drawLine(0, 11, 128, 11); 
      
      u8g2.setFont(u8g2_font_helvB10_tf); 
      u8g2.setCursor(0, 27); u8g2.print(currentWeatherTemp);
      
      u8g2.setFont(u8g2_font_helvR08_tf); 
      u8g2.setCursor(65, 27); 
      const char* daysOfWeek[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
      u8g2.print(daysOfWeek[now.dayOfTheWeek()]);

      u8g2.setFont(u8g2_font_open_iconic_weather_4x_t);
      u8g2.drawGlyph(0, 62, weatherIconCode); 
      
      u8g2.setFont(u8g2_font_logisoso24_tn); 
      u8g2.setCursor(38, 64); 
      if(now.hour()<10) u8g2.print('0'); u8g2.print(now.hour()); u8g2.print(":");
      if(now.minute()<10) u8g2.print('0'); u8g2.print(now.minute());
    }
    u8g2.sendBuffer();
  }
};