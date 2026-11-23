
#include <Arduino.h>
#include <WiFi.h>
#include "internet.h"

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_client_id = "senai134_esp_kaua";
const char *mqtt_topic_sub = "senai134/DevKp/pub";
const char *mqtt_topic_pub = "senai_duoKP/pub";

WiFiClient espClient;
PubSubClient client(espClient);

void conectaMqtt();
void retornoMqtt(char *topic, byte *payload, unsigned int length);
void atualizarLED();

#define pinDHT 4
#define tipoDHT DHT22
#define pinLDR 32
#define pinLED 2

DHT medidor(pinDHT, tipoDHT);

float tempC_anterior = 0.0;
float umidade_anterior = 0.0;

unsigned long tempoAnterior = 0;
const long intervaloEnvio = 3000;

unsigned long tempoAnteriorPisca = 0;
const long intervaloPisca = 500;

String estadoLED = "desliga";
bool estadoPinoLed = false;

void setup()
{
  Serial.begin(115200);

  medidor.begin();

  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);

  pinMode(pinLDR, INPUT);

  conexaoWifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(retornoMqtt);
}

void loop()
{
  reconexaoWifi();

  if (!client.connected())
  {
    conectaMqtt();
  }

  client.loop();

  atualizarLED();

  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnterior >= intervaloEnvio)
  {
    tempoAnterior = tempoAtual;

    float tempC = medidor.readTemperature();
    float umi = medidor.readHumidity();

    int leituraLDR = analogRead(pinLDR);
    int luminosidade = map(leituraLDR, 0, 4095, 0, 100);

    if (Serial.available() > 0)
    {
      String textoDigitado = Serial.readStringUntil('\n');
      textoDigitado.trim();

      if (textoDigitado.length() > 0)
      {
        JsonDocument doc;

        doc["disp"] = "esp-kaua";
        doc["tempC"] = tempC;
        doc["umidade"] = umi;
        doc["luminosidade"] = luminosidade;
        doc["msg"] = textoDigitado;

        String StringJson;
        serializeJson(doc, StringJson);

        if (client.connected())
        {
          client.publish(mqtt_topic_pub, StringJson.c_str());

          Serial.print("Enviado: ");
          Serial.println(StringJson);
        }
      }
      else
      {
        Serial.println("Mensagem vazia");
      }
    }

    tempC_anterior = tempC;
    umidade_anterior = umi;
  }

  atualizarLED();
}

void atualizarLED()
{
  if (estadoLED == "liga")
  {
    estadoPinoLed = true;
    digitalWrite(pinLED, HIGH);
  }
  else if (estadoLED == "desliga")
  {
    estadoPinoLed = false;
    digitalWrite(pinLED, LOW);
  }
  else if (estadoLED == "pisca")
  {
    unsigned long agora = millis();

    if (agora - tempoAnteriorPisca >= intervaloPisca)
    {
      tempoAnteriorPisca = agora;

      estadoPinoLed = !estadoPinoLed;
      digitalWrite(pinLED, estadoPinoLed);
    }
  }
}

void conectaMqtt()
{
  if (!client.connected())
  {
    Serial.println("Conectando ao MQTT");

    if (client.connect(mqtt_client_id))
    {
      Serial.println("Conectado com Sucesso!");
      client.subscribe(mqtt_topic_sub);
    }
    else
    {
      Serial.println("Não houve Conexão");
      Serial.println(client.state());
    }
  }
}

void retornoMqtt(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensagem recebida em ");
  Serial.print(topic);
  Serial.print(": ");

  String msgRecebida = "";

  for (int i = 0; i < length; i++)
  {
    msgRecebida += (char)payload[i];
  }

  Serial.println(msgRecebida);

  JsonDocument doc;

  DeserializationError erro = deserializeJson(doc, msgRecebida);

  if (erro)
  {
    msgRecebida.trim();
    msgRecebida.toLowerCase();

    if (msgRecebida == "liga" || msgRecebida == "desliga" || msgRecebida == "pisca")
    {
      estadoLED = msgRecebida;
      Serial.println(estadoLED);
      return;
    }

    Serial.print("erro ao decodificar o JSON: ");
    Serial.println(erro.c_str());

    return;
  }

  if (!doc["disp"].isNull())
  {
    const char *dispositivo = doc["disp"];
    float tempC = doc["tempC"];
    float umi = doc["umidade"];
    int lum = doc["luminosidade"];

    Serial.println("dados decodificados");

    Serial.print("esp: ");
    Serial.println(dispositivo);

    Serial.print("Temp: ");
    Serial.println(tempC);

    Serial.print("Umidade: ");
    Serial.println(umi);

    Serial.print("Luminosidade: ");
    Serial.println(lum);

    Serial.println("-------------------------------");
  }

  if (!doc["msg"].isNull())
  {
    String comando = doc["msg"].as<String>();
    comando.trim();
    comando.toLowerCase();

    if (comando == "liga" || comando == "desliga" || comando == "pisca")
    {
      estadoLED = comando;
      Serial.print("Estado do LED: ");
      Serial.println(estadoLED);
    }
  }
}
