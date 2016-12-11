
/**
 * Bot.cpp - Toby client library
 * Created by Gabriel Garcia, November 7, 2016.
 */

#include "Arduino.h"
#include "Bot.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


const char* TOBY_URL = "toby.cloud";
const int TOBY_PORT = 444;

// Called when packet received from server
void mqttCallback(char* topic, byte* payload, unsigned int length) {

  Serial.println("mqttCallback()\n");
  Serial.print(topic);
  Serial.print(": ");
  String jsonString = "";
  for (int i = 0; i < length; i++) {
    jsonString += (char)payload[i];
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonString);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
}


Bot::Bot(const char* netId, const char* netSk, const char* botId, const char* botSk) {
  this->_id = botId;
  this->_sk = botSk;
  this->_netId = netId;
  this->_netSk = netSk;
  WiFiClient espClient;
  PubSubClient client(espClient);
  _mqttClient = client;
}

void Bot::setup() {
  Serial.println("bot setup: " + String(TOBY_URL) + " " + String(TOBY_PORT));
  this->_mqttClient.setServer(TOBY_URL, TOBY_PORT);
  this->_mqttClient.setCallback(mqttCallback);
  _setupWifi();
}

void Bot::loop() {
  while (!this->_mqttClient.connected()) {
    this->_reconnect();
    delay(1000);
  }
  this->_mqttClient.loop();
}

bool Bot::connected() {
  return this->_mqttClient.connected();
}


void Bot::follow(String tag, String ack) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& tags = root.createNestedArray("tags");
    root["ack"] = "followed";
    tags.add(tag);
    this->_publish("follow", root);
}

void Bot::sendMessage(String m, String tag, String ack) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonObject& payload = root.createNestedObject("payload");
    JsonArray& tags = root.createNestedArray("tags");
    tags.add(tag);
    payload["message"] = m;
    root["tags"] = tags;
    root["ack"] = ack;
    this->_publish("send", root);
}

void Bot::_publish(String endpoint, JsonObject &message) {
    char pubBuf[200];
    message.printTo(pubBuf, sizeof(pubBuf));
    String topic = "server/" + String(_id) + "/" + endpoint;
    char topicBuf[topic.length()+1];
    topic.toCharArray(topicBuf, topic.length()+1);
    _mqttClient.publish(topicBuf, pubBuf);
}


void Bot::_setupWifi() {
  delay(10);
  // Connect to Wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(_netId);
  WiFi.begin(_netId, _netSk);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void Bot::_reconnect() {
  // Loop until we're reconnected
  while (!_mqttClient.connected()) {
    Serial.print("Attempting MQTT connection... " + String(this->_id) + " " + String(this->_sk));
    if (_mqttClient.connect(_id, _id, _sk)) {
      Serial.println("connected");
      this->_subscribeToBotData(); // subscribe to messages sent to bot
    } else {
      Serial.print("failed, rc=");
      Serial.print(_mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void Bot::_subscribeToBotData() {
    String sub = "client/" + String(_id);
    char subBuf[sub.length()+1];
    sub.toCharArray(subBuf, sub.length()+1);
    _mqttClient.subscribe(subBuf);
}
