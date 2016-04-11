//
// This sketch uses an ESP8266-based Adafruit Feather Huzzah for measuring
// temperature and humidity with a DHT11 sensor
// The values are sent as feeds to an Adafruit IO account via client REST calls
//
// To save power, deep sleep from the ESP8266 library is used
// To wake up the board a signal on pin 16 is sent to the reset input
// (so you need a wire between D16 and the reset pin)
// Note! The connection D16 -> Reset has to be removed when uploading the sketch!
// All activity happens in the setup() method as the board is reset after sleeping
//

#include <ESP8266WiFi.h>
#include "Adafruit_IO_Client.h"

// Your personal WiFi- and Adafruit IO credentials
// Should define WLAN_SSID, WLAN_PASS and AIO_KEY
#include "WIFI_and_Adafruit_IO_parameters.h"

// Sensor setup
#include <DHT.h>
#define DHTPIN 12       // Digital input pin for DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi and Adafruit IO setup
WiFiClient client;
Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);
Adafruit_IO_Feed humidityFeed = aio.getFeed("garageHumidity");
Adafruit_IO_Feed temperatureFeed = aio.getFeed("garageTemperature");

#define SLEEP_SECONDS 600

//
// Method for converting a float to a string as there
// seems to be a bug in the Adafruit_Client which returns
// errors when sending floats directly 
char *ftoa(char *buffer, float f)
{ 
  char *returnString = buffer;
  long integerPart = (long)f;
  itoa(integerPart, buffer, 10);
  while (*buffer != '\0') buffer++;
  *buffer++ = '.';
  long decimalPart = abs((long)((f - integerPart) * 100));
  itoa(decimalPart, buffer, 10);
  return returnString;
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

  char temperature_str[20];
  ftoa(temperature_str,temperature);
  if (temperatureFeed.send(temperature_str))
  {
    Serial.println(temperature_str);
    Serial.println("Sent temperature ok");
  }
  else
  {
    Serial.println("Failed sending temperature");
  }

  char humidity_str[20];
  ftoa(humidity_str,humidity);
  if (humidityFeed.send(humidity_str))
  {
    Serial.println(humidity_str);
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
  setupWiFi();
  
  aio.begin();

  dht.begin();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  sendDataToAdafruitIO(t,h);

  // Put the board to deep sleep to save power. Will send a signal on D16 when it is time to wake up.
  // Thus, connect D16 to the reset pin. After reset, setup will be executed again.
  Serial.print("Going to deep sleep for ");
  Serial.print(SLEEP_SECONDS);
  Serial.println(" seconds");
  ESP.deepSleep(SLEEP_SECONDS * 1000000);

}

void loop() 
{
  // nothing to do here as setup is called when waking up after deep sleep
}

