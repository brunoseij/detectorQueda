#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define TIMEOUT_EVENTO_QUEDA 1000
#define TIMEOUT_WIFI 30000

#define WEBSITE "maker.ifttt.com"
#define SSID "mySSID"
#define PASSWORD "password"

static const uint8_t pinSensorImpacto = 2; // D4
static const uint8_t pinLed = 14;        // D5
static const uint8_t pinBotao = 12;     // D6
static const uint8_t pinBuzzer = 0;      // D3

Adafruit_MPU6050 mpu;
WiFiClient client;

bool Emergencia = false;
bool Notificar = false;
int timerQueda = 0;
int timerWifi = 0;

void ICACHE_RAM_ATTR isr()
{
  Emergencia = !Emergencia;
  if (Emergencia)
  {
    Notificar = true;
  }
}

void setPins()
{
  pinMode(pinLed, OUTPUT);
  pinMode(pinSensorImpacto, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinBotao, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinBotao), isr, FALLING);
}

void initMPU()
{
  while (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip. Trying again");
  }
  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
}

void initWifi()
{
  Serial.print("Connecting to " + *SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Credentials accepted! Connected to wifi\n ");
  Serial.println("");
}

bool detecQueda()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if (a.acceleration.x >= 17)
  {
    delay(50);
    mpu.getEvent(&a, &g, &temp);
    if (a.acceleration.x >= 17)
    {
      Serial.println(a.acceleration.x);
      return true;
    }
  }
  if (a.acceleration.y >= 17)
  {
    Serial.println("ativou no y");
    delay(50);
    mpu.getEvent(&a, &g, &temp);
    if (a.acceleration.y >= 17)
    {
      return true;
    }
  }
  if (a.acceleration.z >= 17)
  {
    Serial.println("ativou no z");
    delay(50);
    mpu.getEvent(&a, &g, &temp);
    if (a.acceleration.z >= 17)
    {
      return true;
    }
  }
  return false;
}

bool timeout(int timer, int timeout)
{
  return ((timer + timeout) >= millis());
}

void setup()
{
  Serial.begin(9600);
  setPins();
  initMPU();
  initWifi();
}

void loop()
{
  if (timerWifi && timeout(timerWifi, TIMEOUT_WIFI))
  {
    client.stop();
    timerWifi = 0;
  }

  if (Emergencia)
  {
    digitalWrite(pinLed, HIGH);
    tone(pinBuzzer, 440);
    if (Notificar)
    {
      if (client.connect(WEBSITE, 80))
      {
        Serial.println("WiFi Client connected ");
        client.print(String("POST ") +
                     "/trigger/<EVENT>/with/key/<PERSONAL_KEY>" + " HTTP/1.1\r\n" + "Host: " + WEBSITE + "\r\n" + "Connection: close\r\n\r\n");
        Notificar = false;
        timerWifi = millis();
      }
      else
      {
        Serial.print("couldnt connect to IFTTT\n");
      }
    }
  }
  else
  {
    digitalWrite(pinLed, LOW);
    noTone(pinBuzzer);
  }

  if (detecQueda())
  {
    timerQueda = millis();
    while (not timeout(timerQueda, TIMEOUT_EVENTO_QUEDA))
    {
      if (digitalRead(pinSensorImpacto) == LOW)
      {
        Emergencia = true;
        Notificar = true;
        break;
      }
    }
  }
}
