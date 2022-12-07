#include "config.h"

#include <Adafruit_BME280.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <Wire.h>


Adafruit_BME280 bme;
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish bme280 = Adafruit_MQTT_Publish(&mqtt, "office/sensor/bme_280");

void setup() {
  Serial.begin(9600);

  bme_init();
  wifi_connect();
}

void loop() {
  mqtt_connect();

  bme_publish();
  delay(10000);
}

void bme_init() {
  if (!bme.begin(0x76)) {
    Serial.printf("Could find BME280 sensor at I2C address 0x76.\n");
    exit(0);
  }
}

void bme_publish() {
  Serial.printf("Publishing BME280 sensor data.\n");

  char buf[100];

  sprintf(buf, "{\"altitude\": %f, \"humidity\": %f, \"pressure\": %f, \"temperature\": %f}",
          bme.readAltitude(SEALEVELPRESSURE_HPA),
          bme.readHumidity(),
          bme.readPressure(),
          bme.readTemperature());

  if (!bme280.publish(buf)) {
    Serial.println("Could not publish!");
  }
}

void wifi_connect() {
  Serial.printf("Connecting to WiFi network %s", WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf(".");
  }

  Serial.printf("\nConnected to WiFi.\n");
  Serial.printf("IP-address: %s\n", WiFi.localIP().toString().c_str());
}

void mqtt_connect() {
  if (mqtt.connected()) {
    return;
  }

  Serial.printf("Connecting MQTT broker %s...\n", MQTT_SERVER);

  uint8_t retries = 3;

  while (mqtt.connect() != 0) {
    Serial.printf("Retrying...\n");
    mqtt.disconnect();
    delay(5000);

    if (--retries == 0) {
      Serial.printf("Could not connect.\n");

      exit(0);
    }
  }

  Serial.printf("Connected to MQTT broker.\n");
}