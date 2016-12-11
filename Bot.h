/**
 * Bot.h - Toby client library
 * Created by Gabriel Garcia, November 7, 2016.
 */

#ifndef BOT_H
#define BOT_H

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


class Bot {
  public:
    Bot(const char*, const char*, const char*, const char*);
    void setup();
    void loop();
    bool connected();
    void follow(String tag, String ack);
    void sendMessage(String, String, String);

  private:
    PubSubClient _mqttClient;
    const char* _netId;
    const char* _netSk;
    const char* _id;
    const char* _sk;
    void _setupWifi();
    void static _mqttCallback(char*, byte*, unsigned int);
    void _reconnect();
    void _subscribeToBotData();
    void _publish(String, JsonObject&);
};

#endif
