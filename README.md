# Controle de LED e Leitura de Sensores via MQTT (ESP32)

Projeto desenvolvido para **ESP32** que se conecta a um **Broker MQTT** para controlar um LED remotamente (acender, apagar ou piscar) a partir de um aplicativo no celular ou tablet, além de realizar a leitura de temperatura, umidade e luminosidade.

---

## Funcionalidades

- **Controle remoto do LED via MQTT:** acende, apaga ou pisca o LED (intervalo de 500 ms sem bloquear o código, utilizando `millis()`).
- **Compatibilidade com Texto e JSON:** aceita comandos enviados tanto em texto puro quanto dentro de um objeto JSON.
- **Leitura de sensores:** coleta dados de temperatura e umidade (**DHT22**) e luminosidade (**LDR**).
- **Publicação via Monitor Serial:** ao digitar uma mensagem no Monitor Serial, o ESP32 publica um JSON contendo a mensagem e as leituras atuais dos sensores no tópico MQTT.
- **Reconexão automática:** mantém a conexão ativa com o Wi-Fi e com o Broker MQTT.

---

## Hardware Utilizado

| Componente | Pino (GPIO) | Função |
| :--- | :---: | :--- |
| **ESP32** | — | Microcontrolador principal |
| **LED** (ou LED interno) | `2` | Atuador controlado via MQTT |
| **Sensor DHT22** | `4` | Leitura de temperatura (°C) e umidade (%) |
| **Sensor LDR** | `32` | Leitura de luminosidade (mapeada de 0 a 100%) |

---

## Bibliotecas Necessárias

Antes de compilar o projeto (Arduino IDE ou PlatformIO), instale as seguintes bibliotecas:

- `PubSubClient` (Nick O'Leary)
- `ArduinoJson` (Benoit Blanchon — v7+)
- `DHT sensor library` (Adafruit)

> **Nota:** O projeto também utiliza o arquivo auxiliar local `internet.h`, responsável pelas funções `conexaoWifi()` e `reconexaoWifi()`.

---

## Configurações MQTT

- **Broker:** `broker.emqx.io`
- **Porta:** `1883`
- **Client ID:** `senai134_esp_kaua`
- **Tópico de Assinatura (Subscribe):** `senai134/DevKp/pub`
- **Tópico de Publicação (Publish):** `senai_duoKP/pub`

---

## Como Controlar pelo Celular ou Tablet

Utilize um aplicativo cliente MQTT (como **IoT MQTT Panel**, **MQTT Dash** ou **MyMQTT**), conecte-se ao broker `broker.emqx.io` na porta `1883` e crie 3 botões configurados para publicar no tópico `senai134/DevKp/pub`.

Os comandos podem ser enviados em **texto simples** ou em **formato JSON**:

| Ação do LED | Payload (Texto) | Payload (JSON) |
| :--- | :--- | :--- |
| **Acender** | `liga` | `{"msg": "liga"}` |
| **Apagar** | `desliga` | `{"msg": "desliga"}` |
| **Piscar (500 ms)** | `pisca` | `{"msg": "pisca"}` |

---

## Formato dos Dados Publicados

Quando uma mensagem é enviada pelo Monitor Serial (baud rate `115200`), o ESP32 publica no tópico `senai_duoKP/pub` um JSON no seguinte formato:

```json
{
  "disp": "esp-kaua",
  "tempC": 25.4,
  "umidade": 60.2,
  "luminosidade": 78,
  "msg": "sua mensagem aqui"
}
