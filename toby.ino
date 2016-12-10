/**
 * TOBY MQTT CLIENT EXAMPLE
 *
 * This example shows how to connect to and use Toby on an Arduino. Just
 * replace the network and bot credentials with your own, and set up any
 * logic you want in the onMessageCallback.
 *
 * TODO: abstract this into its own library
 *
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
const char* netId = "";
const char* netSk = "";

// Update these values with valid bot ID and SK (standard only)
const char* botId = "";
const char* botSk = "";


void setup() {
  Serial.begin(115200);
  setupToby();
}

void loop() {
  tobyLoop();
}

// Called when we receive a Toby message
// This is where all logic should be implemented
void onMessageCallback(JsonObject& message) {
  Serial.println("onMessageCallback()");

  JsonObject& payload = message["payload"];
  JsonArray& tags = message["tags"];
  String ack = message["ack"];
  String from = message["from"];
  String m = payload["message"];
  String tag = tags.get(0);

  Serial.println("Message: " + m);
  Serial.println("From " + from);
  Serial.println("Ack: " + ack);
  Serial.println("Tag: " + tag);

  String response = "what?";

  if (m == "ping")
    response = "pong";

  sendMessage(response, ack, "");
}




// TOBY ////////////////////////////////////////////////////////////////////

const char* MQTT_URL = "toby.cloud";
const int MQTT_PORT  = 444;
const int B_LED = 2; // built in blue led

WiFiClient espClient;
PubSubClient client(espClient);

void tobyLoop() {
  if (!client.connected()) {
    reconnect();
    delay(1000);
  }
  client.loop();
}

void setupToby() {
  setupWifi();
  client.setServer(MQTT_URL, MQTT_PORT);
  client.setCallback(mqttCallback);
}

// Connect to WIFI
void setupWifi() {
  delay(10);
  // Connect to Wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(netId);
  WiFi.begin(netId, netSk);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  flash(B_LED);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// Establish connection to Toby
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(botId, botId, botSk)) {
      Serial.println("connected");
      flash(B_LED);
      subscribeToBotData(); // subscribe to messages sent to bot
      follow(botId);             // follow #<botId>
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// SUBSCRIBE to bot data
void subscribeToBotData() {
    String sub = "client/" + String(botId);
    char subBuf[sub.length()+1];
    sub.toCharArray(subBuf, sub.length()+1);
    client.subscribe(subBuf);
}

// PUBLISH to mqtt topic
void publish(String endpoint, JsonObject &message) {
    char pubBuf[200];
    message.printTo(pubBuf, sizeof(pubBuf));
    String topic = "server/" + String(botId) + "/" + endpoint;
    char topicBuf[topic.length()+1];
    topic.toCharArray(topicBuf, topic.length()+1);
    client.publish(topicBuf, pubBuf);
}

// Follow one tag
void follow(String tag) {
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      JsonArray& tags = root.createNestedArray("tags");
      root["ack"] = "followed";
      tags.add(tag);
      publish("follow", root);
}

// Simple send message function
void sendMessage(String m, String tag, String ack) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonObject& payload = root.createNestedObject("payload");
    JsonArray& tags = root.createNestedArray("tags");
    tags.add(tag);
    payload["message"] = m;
    root["tags"] = tags;
    root["ack"] = ack;
    publish("send", root);
}

// Called when packet received from server
void mqttCallback(char* topic, byte* payload, unsigned int length) {

  Serial.println("");
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

  // execute onMessageCallback
  onMessageCallback(root);
}

// flash an LED
void flash(int pin) {
  for (int i=0; i<20; i++) {
    if (i%2==0) {
      digitalWrite(pin,0);
    } else {
      digitalWrite(pin,1);
    }
    delay(50);
  }
}
