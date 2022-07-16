#include "Secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
 

 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/publish"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/subscribe"
 
float h=0 ;
float t=0;
 
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["LED"] = h;
  doc["RELAY"] = t;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
void messageHandler(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  StaticJsonDocument<200> doc;
  deserializeJson(doc, message);
  String payload = doc["message"];//const char* payload = doc["message"];
  Serial.println("Payload is ");
  Serial.println(payload);
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)message[i]);
//    messageTemp += (char)message[i];
//  }
  Serial.println();
  if (String(topic) == AWS_IOT_SUBSCRIBE_TOPIC) {
    Serial.print("Changing output to ");
    //digitalWrite(LED_BUILTIN, HIGH);
    if(payload == "on"){
      Serial.println("on");
      h=1;
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if(payload == "off"){
      Serial.println("off");
      h=0;
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}
 
//void messageHandler(char* topic, byte* payload, unsigned int length)
//{
//  Serial.print("incoming: ");
//  Serial.println(topic);
// 
//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
//  Serial.println(message);
//}
 
void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  connectAWS();
}
 
void loop()
{
 
 
  if (isnan(h) || isnan(t) )  // Check if any reads failed and exit early (to try again).
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
 
  publishMessage();
  client.loop();
  delay(1000);
}
