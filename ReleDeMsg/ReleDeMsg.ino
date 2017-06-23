/*
  Exemplo básico de conexão a Konker Plataform via MQTT, 
  baseado no https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_auth/mqtt_auth.ino. 
  Este exemplo se utiliza das bibliotecas do ESP8266 programado via Arduino IDE 
  (https://github.com/esp8266/Arduino) e a biblioteca PubSubClient que pode ser 
  obtida em: https://github.com/knolleary/pubsubclient/
*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>

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

//Criando os objetos de conexão com a rede e com o servidor MQTT.
WiFiClient espClient;
PubSubClient client(espClient);

//Criando a função de callback
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
    // Criando um ID randomico (Nota: IDs iguais causam desconexao no Mosquito)
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Tentando conectar
    if (client.connect(clientId.c_str()), USER, PWD) {
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
  //Gerando uma semente aleatoria baseada no relogio
  randomSeed(micros());
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
  client.loop();
}

