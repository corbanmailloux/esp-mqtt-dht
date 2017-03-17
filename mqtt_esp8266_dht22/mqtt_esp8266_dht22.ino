/*
 * ESP8266 MQTT DHT for Home Assistant.
 */

// Depends on the following Arduino libraries:
// - Adafruit Unified Sensor Library: https://github.com/adafruit/Adafruit_Sensor
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library

// Set configuration options for WiFi, MQTT, and DHT in the following file:
#include "config.h"

// WiFi
#include <ESP8266WiFi.h>
const char* wifiSSID = CONFIG_WIFI_SSID;
const char* wifiPassword = CONFIG_WIFI_PASS;
WiFiClient espClient;

// MQTT
// http://pubsubclient.knolleary.net/
#include <PubSubClient.h>
const char* mqttServer = CONFIG_MQTT_HOST;
const char* mqttUsername = CONFIG_MQTT_USER;
const char* mqttPassword = CONFIG_MQTT_PASS;
const char* mqttClientId = CONFIG_MQTT_CLIENT_ID; // Must be unique on the MQTT network
PubSubClient client(espClient);
// MQTT Topics
const char* temperatureTopic = CONFIG_MQTT_TOPIC_TEMP;
const char* humidityTopic = CONFIG_MQTT_TOPIC_HUMID;

// DHT
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTTYPE DHT22 // Update this to match your DHT type
const int dhtPin = CONFIG_DHT_PIN;
const long sampleDelay = CONFIG_DHT_SAMPLE_DELAY;
DHT_Unified dht(dhtPin, DHTTYPE);


const int txPin = 1; // On-board blue LED
unsigned long lastSampleTime = 0;

void setup() {
  // pinMode(txPin, OUTPUT);
  // digitalWrite(txPin, HIGH); // Turn off the on-board LED

  // Serial.begin(115200);
  setupWifi();
  client.setServer(mqttServer, 1883);

  // Set up the DHT sensor
  dht.begin();

  // Print the sensor details
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
}

void setupWifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiSSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqttClientId, mqttUsername, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  float humidity, fahrenheit;

  if (currentMillis - lastSampleTime >= sampleDelay) {
    lastSampleTime = currentMillis;


    sensors_event_t event;  
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println("Error reading temperature!");
    }
    else {
      fahrenheit = cToF(event.temperature);
      client.publish(temperatureTopic, String(fahrenheit).c_str());

      Serial.print("Temperature: ");
      Serial.print(fahrenheit);
      Serial.println(" *F");
    }
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println("Error reading humidity!");
    }
    else {
      humidity = event.relative_humidity;
      client.publish(humidityTopic, String(humidity).c_str());

      Serial.print("Humidity: ");
      Serial.print(event.relative_humidity);
      Serial.println("%");
    }
  }
}

//Celsius to Fahrenheit conversion
float cToF(float celsius)
{
  return 1.8 * (float)celsius + 32;
}
