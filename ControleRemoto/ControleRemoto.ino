/*
  Exemplo básico de conexão a Konker Plataform via MQTT, 
  baseado no https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_auth/mqtt_auth.ino. 
  Este exemplo se utiliza das bibliotecas do ESP8266 programado via Arduino IDE 
  (https://github.com/esp8266/Arduino) e a biblioteca PubSubClient que pode ser 
  obtida em: https://github.com/knolleary/pubsubclient/
*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 

// Vamos primeiramente conectar o ESP8266 com a rede Wireless (mude os parâmetros abaixo para sua rede).

// Dados da rede WiFi
const char* ssid = "";
const char* password = "";

// Dados do servidor MQTT
const char* mqtt_server = "";
const char* PUB = "";
const char* SUB = "";
const char* USER = "";
const char* PWD = "";

//Variaveis gloabais desse codigo
char bufferJ[256];
char *mensagem;

//Variaveis de leitura serial
int blue;
int red;

//Vamos criar uma funcao para formatar os dados no formato JSON
char *jsonMQTTmsgDATA(const char *device_id, const char *metric, int value)
  {
      StaticJsonBuffer<200> jsonMQTT;
      JsonObject& jsonMSG = jsonMQTT.createObject();
        jsonMSG["deviceId"] = device_id;
        jsonMSG["metric"] = metric;
        jsonMSG["blue"] = value;
        jsonMSG.printTo(bufferJ, sizeof(bufferJ));
        return bufferJ;
  }

//Criando os objetos de conexão com a rede e com o servidor MQTT.
WiFiClient espClient;
PubSubClient client(espClient);

//Criando a funcao de callback
//Essa funcao eh rodada quando uma mensagem eh recebida via MQTT.
//Nesse caso ela eh muito simples: imprima via serial o que voce recebeu
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Entra no Loop ate estar conectado
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Usando um ID unico (Nota: IDs iguais causam desconexao no Mosquito)
    // Tentando conectar
    if (client.connect(USER, USER, PWD)) {
      Serial.println("connected");
      // Subscrevendo no topico esperado
      client.subscribe(SUB);
    } else {
      Serial.print("Falhou! Codigo rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      // Esperando 5 segundos para tentar novamente
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  // Agora vamos nos conectar em uma rede Wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Imprimindo pontos na tela ate a conexao ser estabelecida!
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereco de IP: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  //Configurando a porta Serial e escolhendo o servidor MQTT
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  //O programa em si eh muito simples: 
  //se nao estiver conectado no Broker MQTT, se conecte!
  if (!client.connected()) {
    reconnect();
  }
  while (Serial.available() > 0) {
    if (Serial.read() == 'b') blue = Serial.parseInt();    
    if (Serial.read() == '\n') 
        {
          Serial.println(blue);
          mensagem = jsonMQTTmsgDATA("Dispositivo01", "colors", blue);
          client.publish(PUB, mensagem); 
        }
  }
  client.loop();
}

