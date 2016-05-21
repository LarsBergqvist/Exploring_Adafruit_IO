//
// This sketch uses an ESP8266-based Adafruit Feather Huzzah for measuring
// temperature and humidity with a DHT22 sensor
// The values are sent as feeds to an Adafruit IO account via Adafruit MQTT publish calls
// SSL with WiFiClientSecure is used for encrypting the data sent to Adafruit IO
//
// No deep sleep/reset is used as my Huzzah tends to freeze in deep sleep mode
// every other day.
//

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Your personal WiFi- and Adafruit IO credentials
// Should define WLAN_SSID, WLAN_PASS and AIO_KEY and AIO_USERNAME
#include "WIFI_and_Adafruit_IO_parameters.h"

//
// Adafruit IO setup
//
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883 // Use SSL
//#define AIO_SERVERPORT  1883 // No SSL

//WiFiClient client;      // No SSL
WiFiClientSecure client;  // Use SSL

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/temperatureMQTT";
Adafruit_MQTT_Publish temperaturePub = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);

const char HUMIDITY_FEED[] PROGMEM = AIO_USERNAME "/feeds/humidityMQTT";
Adafruit_MQTT_Publish humidityPub = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_FEED);

//
// Sensor setup
//
#include <DHT.h>
#define DHTPIN 12       // Digital input pin for DHT22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SLEEP_SECONDS 600

void MQTT_connect() 
{
  if (mqtt.connected()) 
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  int8_t ret;
  while ((ret = mqtt.connect()) != 0) 
  { 
    // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Connected!");
}

void sendDataToAdafruitIO(float temperature, float humidity)
{
  unsigned long startTime = millis();

  if (isnan(temperature) || isnan(humidity)) 
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.println("Start sending data to Adafruit IO...");

  MQTT_connect();

  if (temperaturePub.publish(temperature))
  {
    Serial.println(temperature);
    Serial.println("Sent temperature ok");
  }
  else
  {
    Serial.println("Failed sending temperature");
  }

  if (humidityPub.publish(humidity))
  {
    Serial.println(humidity);
    Serial.println("Sent humidity ok");
  }
  else
  {
    Serial.println("Failed sending humidity");
  }

  unsigned long endTime = millis();

  Serial.print("Sending data took ");
  Serial.print((endTime-startTime)/1000.0);
  Serial.println(" second(s)");
}

void setupWiFi()
{
  unsigned long startTime = millis();

  // Setup serial port access.
  Serial.begin(115200);

  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  unsigned long endTime = millis();

  Serial.print("Setting up WiFi took ");
  Serial.print((endTime-startTime)/1000.0);
  Serial.println(" second(s)");

}

void setup() 
{
}

void loop() 
{
  while (WiFi.status() != WL_CONNECTED) 
  {
    setupWiFi();
  }
  
  dht.begin();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  sendDataToAdafruitIO(t,h);

  delay(1000*10);
}

