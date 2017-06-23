// Essa eh uma primeira versao do arquivo contendo as funcoes utilizadas para a comunicacao de dispositivos via MQTT na plataforma
// Konker. O proximo passo serah a tranformacao desse arquivo de funcoes para classes em C++.
// A adocao original dessa estrutura simples em C se deu pela velocidade de prototipacao e facil transformacao para uma estrutura
// de classes em um momento de maior especificidade do projeto.
//
// Responsavel: Luis Fernando Gomez Gonzalez



// Vamos precisar de uma variavel Global para todas as funcoes, um enable para rodar todas as maquinas de estado.
int enable=1;

// Vamos precisar de uma variavel Global para todas voltar os settings para o valor inicial.
int resetsettings=0;

// Vamos precisar de uma variavel Global para avisar que uma configuracao foi feita.
int configured=0;

int senddata=0;

// Trigger para mensagens recebidas
int received_msg=0;

//Trigger da configuracao via Json -- Tambem precisa ser uma variavel global para rodar em duas maquinas de estado.
bool shouldSaveConfig = false;

//Buffer das mensagens MQTT
char bufferJ[256];

//Buffer de entrada MQTT;
char msgBufferIN[2048];
char msgTopic[32];

char device_type[5];
char mqtt_channel_in[32];
char mqtt_channel_out[32];
char mqtt_channel_cmd_in[32];
char mqtt_channel_cmd_out[32];
char ack_topic[32];
char connect_topic[32];
char config_period[5];
int config_period_I;
char adc_type_cfg[4];
char gpio_timer[5];
int gpio_timer_I;

char mqtt_topic_in[32];
char mqtt_topic_out[32];
char mqtt_topic_cmd_in[32];
char mqtt_topic_cmd_out[32];

char BOTtoken_cfg[64];
char BOTname_cfg[32];
char BOTusername_cfg[32];

const char in_modifier[4] = "sub";
const char out_modifier[4] = "pub";

//Opcoes de commandos
const char *LED_ON = "LED ON";
const char *LED_OFF = "LED OFF";
const char *LED_SWITCH = "LED SWITCH";

//LED
int LEDState=0;

Ticker t;
bool marker_work=0;
  
//----------------- Funcao de Trigger para salvar configuracao no FS ---------------------------
void saveConfigCallback() {
  Serial.println("Salvar a Configuracao");
  shouldSaveConfig = true;
}

//----------------- Copiando parametros configurados via HTML ---------------------------
void copyHTMLPar(char api_key[], char mqtt_server[], char mqtt_port[], char mqtt_login[], char mqtt_pass[], char mqtt_channel_in[], char mqtt_channel_out[], char mqtt_channel_cmd_in[], char mqtt_channel_cmd_out[], char BOTtoken_cfg[], char BOTname_cfg[], char BOTusername_cfg[], WiFiManagerParameter custom_api_key, WiFiManagerParameter custom_mqtt_server, WiFiManagerParameter custom_mqtt_port, WiFiManagerParameter custom_mqtt_login, WiFiManagerParameter custom_mqtt_pass, WiFiManagerParameter custom_mqtt_channel_in, WiFiManagerParameter custom_mqtt_channel_out, WiFiManagerParameter custom_mqtt_channel_cmd_in, WiFiManagerParameter custom_mqtt_channel_cmd_out, WiFiManagerParameter custom_BOTtoken_cfg, WiFiManagerParameter custom_BOTname_cfg, WiFiManagerParameter custom_BOTusername_cfg){

  strcpy(api_key, custom_api_key.getValue());
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_login, custom_mqtt_login.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  strcpy(mqtt_channel_in, custom_mqtt_channel_in.getValue());
  strcpy(mqtt_channel_out, custom_mqtt_channel_out.getValue());
  strcpy(mqtt_channel_cmd_in, custom_mqtt_channel_cmd_in.getValue());
  strcpy(mqtt_channel_cmd_out, custom_mqtt_channel_cmd_out.getValue());
  strcpy(BOTtoken_cfg, custom_BOTtoken_cfg.getValue());
  strcpy(BOTname_cfg, custom_BOTname_cfg.getValue());
  strcpy(BOTusername_cfg, custom_BOTusername_cfg.getValue());

}

//----------------- Montando o sistema de arquivo e lendo o arquivo config.json ---------------------------
void spiffsMount(char mqtt_server[], char mqtt_port[], char mqtt_login[], char mqtt_pass[], char device_id[], char device_type[], char api_key[], char mqtt_channel_in[], char mqtt_channel_out[], char mqtt_channel_cmd_in[], char mqtt_channel_cmd_out[], char ack_topic[], char connect_topic[], char config_period[]) {

String config_periodS;
String gpio_timerS;

  if (SPIFFS.begin()) {
    Serial.println("Sistema de Arquivos Montado");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("Arquivo já existente, lendo..");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("Arquivo aberto com sucesso");
        size_t size = configFile.size();
        // Criando um Buffer para alocar os dados do arquivo.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          if (json.containsKey("mqtt_server")) strcpy(mqtt_server, json["mqtt_server"]);
          if (json.containsKey("mqtt_port")) strcpy(mqtt_port, json["mqtt_port"]);
          if (json.containsKey("mqtt_login")) strcpy(mqtt_login, json["mqtt_login"]);
          if (json.containsKey("mqtt_pass")) strcpy(mqtt_pass, json["mqtt_pass"]);
          if (json.containsKey("device_id")) strcpy(device_id, json["device_id"]);
          if (json.containsKey("device_type")) strcpy(device_type, json["device_type"]);
          if (json.containsKey("api_key")) strcpy(api_key, json["api_key"]);
          if (json.containsKey("mqtt_channel_in")) strcpy(mqtt_channel_in, json["mqtt_channel_in"]);
          if (json.containsKey("mqtt_channel_out")) strcpy(mqtt_channel_out, json["mqtt_channel_out"]);
          if (json.containsKey("mqtt_channel_cmd_in")) strcpy(mqtt_channel_cmd_in, json["mqtt_channel_cmd_in"]);
          if (json.containsKey("mqtt_channel_cmd_out")) strcpy(mqtt_channel_cmd_out, json["mqtt_channel_cmd_out"]);
          if (json.containsKey("config_period")) strcpy(config_period, json["config_period"]); 
          if (json.containsKey("adc_type_cfg")) strcpy(adc_type_cfg, json["adc_type_cfg"]); 
          if (json.containsKey("BOTtoken_cfg")) strcpy(BOTtoken_cfg, json["BOTtoken_cfg"]); 
          else strcpy(BOTtoken_cfg, "");
          if (json.containsKey("BOTname_cfg")) strcpy(BOTname_cfg, json["BOTname_cfg"]); 
          else strcpy(BOTname_cfg, "");
          if (json.containsKey("BOTusername_cfg")) strcpy(BOTusername_cfg, json["BOTusername_cfg"]); 
          else strcpy(BOTusername_cfg, "");
          if (json.containsKey("gpio_timer")) strcpy(gpio_timer, json["gpio_timer"]); 
          else strcpy(gpio_timer, "0");
        } 
          else {
          Serial.println("Falha em ler o Arquivo");
        }
      }
    }
  } else {
    Serial.println("Falha ao montar o sistema de arquivos");
  }
  config_periodS = config_period;
  config_period_I = config_periodS.toInt();
  gpio_timerS = gpio_timer;
  gpio_timer_I = gpio_timerS.toInt();

    String SString;
  SString = String(in_modifier) + String("/") + String(api_key) + String("/") + String(mqtt_channel_in);
  SString.toCharArray(mqtt_topic_in, SString.length()+1);
  
  SString = String(out_modifier) + String("/") + String(api_key) + String("/") + String(mqtt_channel_out);
  SString.toCharArray(mqtt_topic_out, SString.length()+1);
    
  SString = String(in_modifier) + String("/") + String(api_key) + String("/") + String(mqtt_channel_cmd_in);
  SString.toCharArray(mqtt_topic_cmd_in, SString.length()+1);
  
  SString = String(out_modifier) + String("/") + String(api_key) + String("/") + String(mqtt_channel_cmd_out);
  SString.toCharArray(mqtt_topic_cmd_out, SString.length()+1);
  

}

//----------------- Salvando arquivo de configuracao ---------------------------
void saveConfigtoFile(char api_key[], char device_id[], char mqtt_server[],char mqtt_port[],char mqtt_login[],char mqtt_pass[], char mqtt_channel_in[], char mqtt_channel_out[], char mqtt_channel_cmd_in[], char mqtt_channel_cmd_out[])
{

  Serial.println("Salvando Configuracao");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_port;
  json["mqtt_login"] = mqtt_login;
  json["mqtt_pass"] = mqtt_pass;
  json["device_id"] = device_id;
  json["device_type"] = device_type;
  json["api_key"] = api_key;
  json["mqtt_channel_in"] = mqtt_channel_in;
  json["mqtt_channel_out"] = mqtt_channel_out;
  json["mqtt_channel_cmd_in"] = mqtt_channel_cmd_in;
  json["mqtt_channel_cmd_out"] = mqtt_channel_cmd_out;
  json["config_period"] = config_period;
  json["adc_type_cfg"] = adc_type_cfg;
  json["BOTtoken_cfg"] = BOTtoken_cfg;
  json["BOTname_cfg"] = BOTname_cfg;
  json["BOTusername_cfg"] = BOTusername_cfg;
  json["gpio_timer"] = gpio_timer;
  
File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Falha em abrir o arquivo com permissao de gravacao");
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

//----------------- Configuracao --------------------------------
//----------------- Criacao da Mensagem Json --------------------------------
char *jsonMQTTmsgDATA(const char *ts, const char *device_id, const char *metric, int value, const char *unit)
  {
      StaticJsonBuffer<200> jsonMQTT;
      JsonObject& jsonMSG = jsonMQTT.createObject();
        jsonMSG["deviceId"] = device_id;
        jsonMSG["metric"] = metric;
        jsonMSG["value"] = value;
        jsonMSG["unit"] = unit;
        jsonMSG["ts"] = ts;
        jsonMSG.printTo(bufferJ, sizeof(bufferJ));
        return bufferJ;
  }

char *jsonMQTTmsgCMD(const char *ts, const char *device_id, const char *cmd)
  {
      StaticJsonBuffer<200> jsonMQTT;
      JsonObject& jsonMSG = jsonMQTT.createObject();
        jsonMSG["deviceId"] = device_id;
        jsonMSG["command"] = cmd;
        jsonMSG["ts"] = ts;
        jsonMSG.printTo(bufferJ, sizeof(bufferJ));
        return bufferJ;
  }

char *jsonMQTTmsgCONNECT(const char *ts, const char *device_id, const char *connect_msg, const char *metric, int value, const char *unit)
  {
      StaticJsonBuffer<200> jsonMQTT;
      JsonObject& jsonMSG = jsonMQTT.createObject();
        jsonMSG["deviceId"] = device_id;
        jsonMSG["connectmsg"] = connect_msg;
        jsonMSG["metric"] = metric;
        jsonMSG["value"] = value;
        jsonMSG["unit"] = unit;
        jsonMSG["ts"] = ts;
        jsonMSG.printTo(bufferJ, sizeof(bufferJ));
        return bufferJ;
  }  
//----------------- Decodificacao da mensagem Json In -----------------------------
char *jsonMQTT_in_msg(const char msg[])
  {
  const char *jdevice_id;
  const char *cmd = "null";
  const char *ts;

  StaticJsonBuffer<2048> jsonBuffer;
  JsonObject& jsonMSG = jsonBuffer.parseObject(msg);
  if (jsonMSG.containsKey("deviceId")) jdevice_id = jsonMSG["deviceId"];
  if (jsonMSG.containsKey("command")) cmd = jsonMSG["command"];
  if (jsonMSG.containsKey("ts")) ts = jsonMSG["ts"];
  char *command = (char*)cmd;
  return command;
  }
  
//----------------- Decodificacao dos dados da mensagem Json In --------------------
char *jsonMQTT_in_data_msg(const char msg[])
  {
  const char *jdevice_id;
  const char *cmd;
  const char *ts;
  const char *dt;
  
  StaticJsonBuffer<2048> jsonBuffer;
  JsonObject& jsonMSG = jsonBuffer.parseObject(msg);
  if (jsonMSG.containsKey("deviceId")) jdevice_id = jsonMSG["deviceId"];
  if (jsonMSG.containsKey("command")) cmd = jsonMSG["command"];
  if (jsonMSG.containsKey("value")) dt = jsonMSG["value"];
  if (jsonMSG.containsKey("ts")) ts = jsonMSG["ts"];
  char *data = (char*)dt;
  return data;
  }
  

//----------------- Decodificacao da mensagem Json CFG -----------------------------
void *jsonMQTT_config_msg(const char msg[])
  {
  int value = 0;
  const char *configs;
  char *conf;
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& jsonMSG = jsonBuffer.parseObject(msg);
  if (jsonMSG.containsKey("config")) 
                                    {
                                      configs = jsonMSG["config"];
                                      conf=strdup(configs);
                                      if (jsonMSG.containsKey("value")) value = jsonMSG["value"];
                                    }
  }

//---------------------------------------------------------------------------



//----------------- Funcao para conectar ao broker MQTT e reconectar quando perder a conexao --------------------------------
int reconnect(PubSubClient client, char id[], const char *mqtt_login, const char *mqtt_pass) {
  int i=0;
  // Loop ate estar conectado
  while (!client.connected()) {
    Serial.print("Tentando conectar no Broker MQTT...");
    // Tentando conectar no broker MQTT (PS.: Nao usar dois clientes com o mesmo nome. Causa desconexao no Mosquitto)
    if (client.connect(mqtt_login, mqtt_login, mqtt_pass)) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println("Tentando conectar novamente em 3 segundos");
      // Esperando 3 segundos antes de re-conectar
      delay(3000);
      if (i==19) break;
      i++;
    }
  }
  return i;
}



//----------------- Funcao para testar as mensagens de entrada ----------------------

char *testCMD(const char msgBufferIN[])
{
  char *cmd;
  String msgString ="null";
  String type;
  char typeC[32];
  int commaIndex;
  
  cmd=jsonMQTT_in_msg(msgBufferIN);
  msgString = String(cmd);
  commaIndex = msgString.indexOf('-');
  type = msgString.substring(0, commaIndex);
  if (commaIndex == -1) 
          {
            type = msgString.substring(0, msgString.length());
          }
  type.toCharArray(typeC,type.length()+1);
  typeC[type.length()+1] = '\0';

  return typeC;
    
}

//-----------------------------------------------------------------------------------
void LEDSwitch(int LED1, int LED2)
{       
   //LEDState = digitalRead(LED1);
   Serial.print("Digital State: ");
   Serial.println(LEDState);
   if (LEDState)
   {
   analogWrite(LED1, 0);
   analogWrite(LED2, 0); 
   }
   else 
   {
   analogWrite(LED1, 1024);
   analogWrite(LED2, 1024);
   }
   LEDState=!LEDState;
 }

void LEDOn(int LED1, int LED2, int pwm)
{
   
   analogWrite(LED1, pwm);
   analogWrite(LED2, pwm); 
   LEDState=1;
}

void LEDOff(int LED1, int LED2)
{
   analogWrite(LED1, 0);
   analogWrite(LED2, 0);
   LEDState=0;
}

void sendMQTTdata(char api_key[], PubSubClient client)
{
  char * mqtt_message_data_timer;
  int gpio_state=0;
  float voltage_adc=0;
  
  if (gpio_timer_I == 1) 
            {
              gpio_state = digitalRead(4);
              mqtt_message_data_timer = jsonMQTTmsgDATA("00000", api_key, "State", gpio_state, "Logic State");
            }
  else  if (gpio_timer_I == 2) 
            {
              gpio_state = digitalRead(5);
              mqtt_message_data_timer = jsonMQTTmsgDATA("00000", api_key, "State", gpio_state, "Logic State");
            }

  else  if (gpio_timer_I == 3) 
            {
              voltage_adc = analogRead(A0)/1024;
              mqtt_message_data_timer = jsonMQTTmsgDATA("00000", api_key, "Voltage", voltage_adc, "milivolts");
            }

 if (gpio_timer_I >0) client.publish(mqtt_topic_out,mqtt_message_data_timer);
}

void TimerMaker()  
{
 marker_work =1; 
}

