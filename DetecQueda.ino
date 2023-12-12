// inclusão de bibliotecas
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define TIMEOUT_NOTIFICACAO 15000

#define WEBSITE "maker.ifttt.com"

// configuracao da rede
#define SSID "nome_rede"
#define PASSWORD "senha_rede"

// definicao das estruturas de dados
typedef struct
{
  bool ativo;
  bool Notificar;
  int timer;
  int timeout;
} evento;

typedef struct
{
  int pino;
  bool ativo;
  int timer;
  int timeout;
} componente;

// inicializacao de variaveis
evento Queda = {false, false, 0, 1000};
evento Conexao = {false, false, 0, 30000};
evento Impacto = {false, false, 0, 1000};
Adafruit_MPU6050 mpu;
WiFiClient client;
componente Buzzer = {15, false, millis(), 300};
componente Led = {14, false, millis(), 1000};
componente SensorImpacto = {2, false, 0, 0};
componente Botao = {12, false, millis(), 0};

// funcao de interrupcao (executada ao pressionar o botao)
void ICACHE_RAM_ATTR isr()
{
  if (timeout(Botao.timer, 400))
  {
    Botao.timer = millis();
    Queda.ativo = !Queda.ativo;
    if (Queda.ativo)
    {
      Queda.Notificar = true;
      return;
    }
    Queda.Notificar = false;
  }
}

// configuracao dos pinos
void setPins()
{
  pinMode(Led.pino, OUTPUT);
  pinMode(SensorImpacto.pino, INPUT);
  pinMode(Buzzer.pino, OUTPUT);
  pinMode(Botao.pino, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Botao.pino), isr, FALLING);
}

// funcao para iniciar o acelerometro
void initMPU()
{
  while (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip. Trying again");
  }
  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
}

// funcao para conectar a rede wi-fi
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

// funcao responsavel pela deteccao de queda do acelerometro
bool detecQueda()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  if (a.acceleration.x <= -6)
  {
    return true;
  }
  if (a.acceleration.y >= 16)
  {
    return true;
  }
  if (a.acceleration.z >= 16)
  {
    return true;
  }
  return false;
}

// funcao utilitaria para controlar tempos do superloop
bool timeout(int timer, int timeout)
{
  if (timer == 0)
    return false;
  return (millis() >= (timer + timeout));
}

// funcao de envio de requisicao para a API de envio de notificacoes
bool enviarNotificacao()
{
  if (client.connect(WEBSITE, 80))
  {
    Serial.println("WiFi Client connected ");
    client.print(String("POST ") +
                 "/trigger/{API}/with/key/{personal_key}" +
                 " HTTP/1.1\r\n" +
                 "Host: " + WEBSITE + "\r\n" +
                 "Connection: close\r\n\r\n");
    return true;
  }
  else
  {
    Serial.print("couldnt connect to IFTTT\n");
    return false;
  }
}

// funcao para piscar o led
void piscarLed()
{
  if (timeout(Led.timer, Led.timeout))
  {
    Led.timer = millis();
    digitalWrite(Led.pino, !digitalRead(Led.pino)); // piscando led
  }
}

// funcao para tocar o alarme
void tocarAlarme()
{
  if (timeout(Buzzer.timer, Buzzer.timeout))
  {
    Buzzer.timer = millis();
    if (Buzzer.ativo)
    {
      noTone(Buzzer.pino);
      Buzzer.ativo = false;
    }
    else
    {
      Buzzer.ativo = true;
      tone(Buzzer.pino, 880);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  setPins();
  digitalWrite(Led.pino, HIGH);
  initMPU();
  initWifi();
}

// loop principal
void loop()
{
  // verificando se expirado os 15 segundos de alerta antes de enviar notificacao
  if (Queda.ativo && timeout(Queda.timer, TIMEOUT_NOTIFICACAO))
  {
    // caso o timer esteja expirado, ligada uma flag para envio de notificacao
    Queda.Notificar = true;
    Queda.timer = 0;
  }

  // verificando se o tempo de conexao com o servidor foi expirado
  if (Conexao.ativo && timeout(Conexao.timer, Conexao.timeout))
  {
    Conexao.ativo = false;
    client.stop();
  }

  // se detectada uma queda cai nessa condicional
  if (Queda.ativo)
  {
    piscarLed();

    if (Conexao.Notificar)
    {
      tocarAlarme();
    }

    if (Queda.Notificar)
    {
      Conexao.Notificar = true;
      if (enviarNotificacao())
      {
        Conexao.ativo = true;
        Conexao.timer = millis();
        Queda.Notificar = false;
      }
    }
  }
  else
  {
    // caso a flag não esteja ativa, desligar o led e o buzzer
    digitalWrite(Led.pino, LOW);
    noTone(Buzzer.pino);
    Conexao.Notificar = false;
  }

  // caso seja detectada uma grande aceleracao, verificar se houve impacto
  if (!Queda.ativo && detecQueda())
  {
    Impacto.timer = millis();
    while (!timeout(Impacto.timer, Impacto.timeout))
    {
      // caso haja impacto, ligar os flags de alerta
      if (digitalRead(SensorImpacto.pino) == LOW)
      {
        Serial.println("Impacto detectado");
        Queda.timer = millis();
        Queda.ativo = true;
        Queda.Notificar = false;
        break;
      }
    }
  }
}
