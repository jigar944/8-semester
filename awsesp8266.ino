
/**
 * This is an example sketch to connect your ESP8266 to the AWS IoT servers.
 * Check http://michaelteeuw.nl for more information.
 *
 * Don't forget to add your certificate and key to the data directory
 * and upload your spiffs (data) folder using the following terminal command:
 *
 * platformio run --target uploadfs
 */

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "FS.h"
#include <WiFiClientSecure.h>
#include <ESP8266AWSImplementations.h>

#include <AmazonIOTClient.h>

// Set your Wifi Network & Password
// And define your AWS endpoint which you can find
// in the AWS IoT core console.
const char* ssid = "Meeting";
const char* password = "0123456789";
const char* AWSEndpoint = "a2x6n31s8egef5-ats.iot.ap-south-1.amazonaws.com";
AmazonIOTClient iotClient;
ActionError actionError;
Esp8266DateTimeProvider dateTimeProvider;
 WiFiClientSecure espClient;

/**
 * Add your ssl certificate and ssl key to the data folder. (You can remove the example files.)
 * After adding the files, upload them to your ESP8266 with the following terminal command:
 *
 * platformio run --target uploadfs
 */
const char* certFile = "/801be94945-certificate.txt";
const char* keyFile = "/801be94945-private.txt";

// The following two variables are used for the example outTopic messages.
unsigned int counter = 0;
unsigned long lastPublished = 0;

/**
 * Callback to handle the incoming MQTT messages.
 */
 void initAWS(){
 iotClient.setAWSRegion("ap-south-1");
  iotClient.setAWSEndpoint("amazonaws.com");
  iotClient.setAWSDomain("a2x6n31s8egef5-ats.iot.ap-south-1.amazonaws.com");
  iotClient.setAWSPath("/things/ESP8266testing/shadow");
  iotClient.setAWSKeyID("AKIAT2UXSSNQT3HG4LIO");
  iotClient.setAWSSecretKey("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");
   iotClient.setWiFiClient(&espClient);
  iotClient.setDateTimeProvider(&dateTimeProvider);
 }
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

/**
 * Function to (re)connect to the MQTT server.
 */

 PubSubClient client(AWSEndpoint, 8883, callback, espClient);

void connectMqtt() {
  // As long as there is no connection, try to connect.
  while (!client.connected()) {
    Serial.print("Connecting to MQTT server ...");

    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    delay(500);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("inTopic");
      client.publish("outTopic", "Connected!");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(". Try again in 3 seconds ...");
      delay(3000);
    }
  }
}

// Initialize espClient and (mqtt) client instances.
// Note that we use `WiFiClientSecure` in stead of `WiFiClient`


/**
 * Setup function.
 */
void setup() {
  Serial.begin(115200);

  // Connect to Wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");

  // Mount file system.
  if (!SPIFFS.begin()) Serial.println("Failed to mount file system");

  // Allows for 'insecure' TLS connections. Of course this is totally insecure,
  // but seems the only way to connect to IoT. So be cautious with your data.
  espClient.setInsecure();

  // Read the SSL certificate from the spiffs filesystem and load it.
  File cert = SPIFFS.open("/801be94945-certificate.pem.crt", "r");
  if (!cert) Serial.println("Failed to open certificate file: " + String(certFile));
  if(espClient.loadCertificate(cert)) Serial.println("Certificate loaded!");

  // Read the SSL key from the spiffs filesystem and load it.
  File key = SPIFFS.open("/801be94945-private.pem.key", "r");
  if (!key) Serial.println("Failed to open private key file: " + String(keyFile));
  if(espClient.loadPrivateKey(key)) Serial.println("Private key loaded!");
  initAWS();
}

/**
 * Main run loop
 */
void loop() {
  // Make sure MQTT is connected and run the `loop` method to check for new data.
  if (!client.connected()) connectMqtt();
  client.loop();
  
  // Publish a message every second
   const char* message="{\"outTopic\" : \"Hello World\"}";
    client.publish("outTopic", message);
     char* result = iotClient.update_shadow(message, actionError);
     Serial.println(result);
    Serial.println("Message published [outTopic] : Hello world");
    lastPublished = millis();
  }
