#pragma once
#include <Arduino.h>
#include "Credentials.h" 

#define BUTTON_PIN 13 
#define RELAY_PIN 5 

#define MAX_ALARMS 5
const int PAUSE_DURATION = 3000;

const unsigned long SCHEDULE_DURATION = 30 * 60 * 1000UL;
const unsigned long TRAIN_FETCH_INTERVAL = 5 * 60 * 1000UL;

struct Alarm {
  bool active;
  int hour;
  int minute;
  int melody; 
  bool sunrise;
  bool days[7]; 
};

struct TrainInfo {
  String text;
};