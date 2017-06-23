// Codigo do LED com ACK em comunicacao MQTT
// Responsavel: Luis Fernando Gomez Gonzalez (luis.gonzalez@inmetrics.com.br)
// Projeto Konker

// Agora usando o gerenciador de conexão WiFiManagerK.

#include <FS.h>  
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManagerK.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoJson.h> 
#include "konker.h"
#include <ESP8266TelegramBOT.h>



extern "C" {
  #include "user_interface.h"
}

// Initialize Telegram BOT
#define BOTtoken ""  //token of TestBOT
#define BOTname ""
#define BOTusername ""
TelegramBOT bot(BOTtoken, BOTname, BOTusername);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

int configurechannel = 0;
int conf_channel = 0;
float voltage;


//Variaveis da Configuracao em Json
char api_key[17];
char device_id[17];
char mqtt_server[64];
char mqtt_port[5];
char mqtt_login[32];
char mqtt_pass[32];
char *mensagemjson;

char mqtt_server_temp[64];
char mqtt_port_temp[5];
char mqtt_login_temp[32];
char mqtt_pass_temp[32];
char mqtt_in_channel_temp[32];
char mqtt_out_channel_temp[32];
char adc_type_cfg_temp[4];

//Variaveis Fisicas
float temperature;

//String de command
String pubString;
char message_buffer[20];
int reconnectcount=0;
char *type;
char typeC[32];
String Stype;
int disconnected=0;

int marked=0;

//Variaveis do ambiente do BOT
bool configure_d = 0;
bool mqtt_server_d = 0;
bool mqtt_port_d = 0;
bool mqtt_login_d = 0;
bool mqtt_password_d = 0;
bool mqtt_in_channel_d = 0;
bool mqtt_out_channel_d = 0;
bool timer_d = 0;
bool timer_seconds_d = 0;
bool change_timer = 0;
bool tipo_d = 0;
bool input_io_d = 0;
bool adc_mode_d = 0;
bool gpio_config_d = 0;
bool gpio_config_d2 = 0;

int gpio_config_pin[5];

//Definindo os objetos de Wifi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

//Ticker
Ticker timer;


//Criando as variaveis dentro do WiFiManager
WiFiManagerParameter custom_api_key("api", "api key", api_key, 17);
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 64);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
WiFiManagerParameter custom_mqtt_login("login", "mqtt login", mqtt_login, 32);
WiFiManagerParameter custom_mqtt_pass("password", "mqtt password", mqtt_pass, 32);
WiFiManagerParameter custom_mqtt_channel_in("channel_input", "mqtt subscription channel (data)", mqtt_channel_in, 32);
WiFiManagerParameter custom_mqtt_channel_out("channel_output", "mqtt publication channel (data)", mqtt_channel_out, 32);
WiFiManagerParameter custom_mqtt_channel_cmd_in("command_channel_input", "mqtt subscription channel (command)", mqtt_channel_cmd_in, 32);
WiFiManagerParameter custom_mqtt_channel_cmd_out("command_channel_output", "mqtt publication channel (command)", mqtt_channel_cmd_out, 32);
WiFiManagerParameter custom_BOTtoken_cfg("BOTtoken_cfg", "Telegram Bot Token", BOTtoken_cfg, 64);
WiFiManagerParameter custom_BOTname_cfg("BOTname_cfg", "Telegram Bot Name", BOTname_cfg, 32);
WiFiManagerParameter custom_BOTusername_cfg("BOTusername_cfg", "Telegram Bot username", BOTusername_cfg, 32);

WiFiManager wifiManager;  

// Funcao de Callback para o MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  int i;
  int state=0;
  
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("] ");
  for (i = 0; i < length; i++) {
    msgBufferIN[i] = payload[i];
    Serial.print((char)payload[i]);
  }
  msgBufferIN[i] = '\0';
  strcpy(msgTopic, topic);
  received_msg = 1;
  Serial.println("");
}

// Setup do Microcontrolador: Vamos configurar o acesso serial, conectar no Wifi, configurar o MQTT e o GPIO do botao
void setup(){
  
  Serial.begin(115200);  
 
//------------------- Montando Sistema de arquivos e copiando as configuracoes  ----------------------
spiffsMount(mqtt_server, mqtt_port, mqtt_login, mqtt_pass, device_id, device_type, api_key, mqtt_channel_in, mqtt_channel_out, mqtt_channel_cmd_in, mqtt_channel_cmd_out, ack_topic, connect_topic, config_period);  

//Criando as variaveis dentro do WiFiManager
WiFiManagerParameter custom_api_key("api", "api key", api_key, 17);
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 64);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
WiFiManagerParameter custom_mqtt_login("login", "mqtt login", mqtt_login, 32);
WiFiManagerParameter custom_mqtt_pass("password", "mqtt pass", mqtt_pass, 32);
WiFiManagerParameter custom_mqtt_channel_in("channel_input", "mqtt channel data input", mqtt_channel_in, 32);
WiFiManagerParameter custom_mqtt_channel_out("channel_output", "mqtt channel data output", mqtt_channel_out, 32);
WiFiManagerParameter custom_mqtt_channel_cmd_in("command_channel_input", "mqtt channel command input", mqtt_channel_cmd_in, 32);
WiFiManagerParameter custom_mqtt_channel_cmd_out("command_channel_output", "mqtt channel command input", mqtt_channel_cmd_out, 32);
WiFiManagerParameter custom_BOTtoken_cfg("BOTtoken_cfg", "Telegram Bot Token", BOTtoken_cfg, 64);
WiFiManagerParameter custom_BOTname_cfg("BOTname_cfg", "Telegram Bot Name", BOTname_cfg, 32);
WiFiManagerParameter custom_BOTusername_cfg("BOTusername_cfg", "Telegram Bot username", BOTusername_cfg, 32);


//------------------- Configuracao do WifiManager K ----------------------
  wifiManager.setTimeout(500);
  wifiManager.setBreakAfterConfig(1);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_api_key);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_login);
  wifiManager.addParameter(&custom_mqtt_pass);
  wifiManager.addParameter(&custom_mqtt_channel_in);
  wifiManager.addParameter(&custom_mqtt_channel_out);
  wifiManager.addParameter(&custom_mqtt_channel_cmd_in);
  wifiManager.addParameter(&custom_mqtt_channel_cmd_out);
  wifiManager.addParameter(&custom_BOTtoken_cfg);
  wifiManager.addParameter(&custom_BOTname_cfg);
  wifiManager.addParameter(&custom_BOTusername_cfg);
  //wifiManager.resetSettings(); //Caso seja necessario resetar os valores para Debug
  

//------------------- Caso conecte, copie os dados para o FS ----------------------
  if(!wifiManager.autoConnect("KonkerConfig")) {
    
    //Copiando parametros  
    copyHTMLPar(api_key, mqtt_server, mqtt_port, mqtt_login, mqtt_pass, mqtt_channel_in, mqtt_channel_out, mqtt_channel_cmd_in, mqtt_channel_cmd_out, BOTtoken_cfg, BOTname_cfg, BOTusername_cfg, custom_api_key, custom_mqtt_server, custom_mqtt_port, custom_mqtt_login, custom_mqtt_pass, custom_mqtt_channel_in, custom_mqtt_channel_out, custom_mqtt_channel_cmd_in, custom_mqtt_channel_cmd_out, custom_BOTtoken_cfg, custom_BOTname_cfg, custom_BOTusername_cfg);
    
    //Salvando Configuracao
    if (shouldSaveConfig) {  
                             saveConfigtoFile(api_key, device_id, mqtt_server, mqtt_port, mqtt_login, mqtt_pass, mqtt_channel_in, mqtt_channel_out, mqtt_channel_cmd_in, mqtt_channel_cmd_out);
                          }
    delay(2500);
    ESP.reset();
    } 

//------------------- Caso tudo mais falhe copie os dados para o FS ----------------------
//Copiando parametros  
  copyHTMLPar(api_key, mqtt_server, mqtt_port, mqtt_login, mqtt_pass, mqtt_channel_in, mqtt_channel_out, mqtt_channel_cmd_in, mqtt_channel_cmd_out, BOTtoken_cfg, BOTname_cfg, BOTusername_cfg, custom_api_key, custom_mqtt_server, custom_mqtt_port, custom_mqtt_login, custom_mqtt_pass, custom_mqtt_channel_in, custom_mqtt_channel_out, custom_mqtt_channel_cmd_in, custom_mqtt_channel_cmd_out, custom_BOTtoken_cfg, custom_BOTname_cfg, custom_BOTusername_cfg);
 
//Salvando Configuracao
if (shouldSaveConfig) {  
  saveConfigtoFile(api_key,device_id, mqtt_server,mqtt_port,mqtt_login,mqtt_pass,mqtt_channel_in, mqtt_channel_out, mqtt_channel_cmd_in, mqtt_channel_cmd_out); 
  }

//------------------- Configurando MQTT e GPIOs ----------------------
  client.setServer(mqtt_server, atol(mqtt_port));
  client.setCallback(callback);
  delay(200);
  if (!client.connected()) {
    disconnected=1;
    reconnectcount = reconnect(client, api_key, mqtt_login, mqtt_pass);
    if (reconnectcount>10) resetsettings=1;
  }
  client.subscribe(mqtt_topic_in);
  client.subscribe(mqtt_topic_cmd_in);
 
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
 
//-----------------------------------------------------------------------------------------


}

 //TelegramBOT bot(BOTtoken_cfg, BOTname_cfg, BOTusername_cfg);
 //TelegramBOT bot(BOTtoken, BOTname, BOTusername);

 //ADC_MODE(ADC_VCC);
 
void Bot_ExecMessages() {
  for (int i = 1; i < bot.message[0][0].toInt() + 1; i++)      {
    bot.message[i][5]=bot.message[i][5].substring(0,bot.message[i][5].length());
    Serial.print("Mensagem: ");
    Serial.println(bot.message[i][5]);

  if (bot.message[i][5].length() > 0)
   {
  
    if (bot.message[i][5] == "\\/voltage") {
      voltage = ESP.getVcc();
      String messageV = String(voltage);
      messageV += " milivolts";
      bot.sendMessage(bot.message[i][4], messageV, "");
    }
    if (bot.message[i][5] == "\\/ip") {
      IPAddress ip = WiFi.localIP();
      String messageIP = "My IP Address is: ";
      String ip_s = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
      messageIP += ip_s;
      bot.sendMessage(bot.message[i][4], messageIP, "");
    }
    if (bot.message[i][5] == "\\/send voltage by mqtt") {
      voltage = analogRead(A0)/1024;
      char * mqtt;
      String messageMQTT_V = "Mensagem enviada para o Broker ";
      messageMQTT_V += String(mqtt_server);
      messageMQTT_V += " no tópico ";
      messageMQTT_V += String(mqtt_topic_out);
      messageMQTT_V += ".";
      bot.sendMessage(bot.message[i][4], messageMQTT_V, "");
      mqtt = jsonMQTTmsgDATA("00000", api_key, "Voltage", voltage, "milivolts");
      client.publish(mqtt_topic_out,mqtt);
    }
    
 if (bot.message[i][5] == "\\/send GPIO 4 by mqtt") {
      int gpio4_state = digitalRead(4);
      char * mqtt_G4;
      String messageMQTT_G4 = "Mensagem enviada para o Broker ";
      messageMQTT_G4 += String(mqtt_server);
      messageMQTT_G4 += " no tópico ";
      messageMQTT_G4 += String(mqtt_topic_out);
      messageMQTT_G4 += ".";
      bot.sendMessage(bot.message[i][4], messageMQTT_G4, "");
      mqtt_G4 = jsonMQTTmsgDATA("00000", api_key, "State", gpio4_state, "Logic State");
      client.publish(mqtt_topic_out,mqtt_G4);
    }

 if (bot.message[i][5] == "\\/send GPIO 5 by mqtt") {
      int gpio5_state = digitalRead(5);
      char * mqtt_G5;
      String messageMQTT_G5 = "Mensagem enviada para o Broker ";
      messageMQTT_G5 += String(mqtt_server);
      messageMQTT_G5 += " no tópico ";
      messageMQTT_G5 += String(mqtt_topic_out);
      messageMQTT_G5 += ".";
      bot.sendMessage(bot.message[i][4], messageMQTT_G5, "");
      mqtt_G5 = jsonMQTTmsgDATA("00000", api_key, "State", gpio5_state, "Logic State");
      client.publish(mqtt_topic_out,mqtt_G5);
    }

if (timer_seconds_d) {
      String timer_sec_2 = bot.message[i][5];
      if (timer_sec_2 == "GPIO4") 
      {
        gpio_timer_I = 1;
        gpio_timer[0] = '1';
      }
      else if (timer_sec_2 == "GPIO5")
      {
        gpio_timer_I = 2;
        gpio_timer[0] = '2';
      }

      else if (timer_sec_2 == "ADC")
      {
        gpio_timer_I = 3;
        gpio_timer[0] = '3';
      }
      
      else
      {
        String message_timer_error2 = "Não entendi o GPIO desejado. Você poderia escrever novamente? (Opções: GPIO4, GPIO5 ou ADC)";
        bot.sendMessage(bot.message[i][4], message_timer_error2, "");
        break;
      }
        String message_timer2_ok = "Novo GPIO registrado: ";
        message_timer2_ok += timer_sec_2;
        bot.sendMessage(bot.message[i][4], message_timer2_ok, "");
        saveConfigtoFile(api_key,device_id, mqtt_server,mqtt_port,mqtt_login,mqtt_pass,mqtt_channel_in, mqtt_channel_out, mqtt_channel_cmd_in, mqtt_channel_cmd_out);
        change_timer =1;
        timer_seconds_d =0;
  }
      

 if (timer_d) {
      String timer_sec = bot.message[i][5];
      int period = timer_sec.toInt();
      if (period<1)
      {
        String message_timer_error = "Não entendi o período desejado. Você poderia escrever novamente o tempo entre mensagens que você gostaria (em segundos, somente números)?";
        bot.sendMessage(bot.message[i][4], message_timer_error, "");
      }
      else 
      {
        String message_timer_ok = "Novo tempo entre mensagens registrado: ";
        message_timer_ok += String(period);
        message_timer_ok += " segundos.";
        timer_sec.toCharArray(config_period, timer_sec.length()+1);
        config_period_I = period;
        bot.sendMessage(bot.message[i][4], message_timer_ok, "");
        String message_timer_ok2 = "Agora, por favor digite o GPIO que deve ser monitorado. (Opções: GPIO4, GPIO5 ou ADC)";
        bot.sendMessage(bot.message[i][4], message_timer_ok2, "");
        timer_d =0;
        timer_seconds_d =1;
      }      
    }

 if (bot.message[i][5] == "\\/timer") {
      String message_timer = "Vamos agora configurar o Timer. Qual o tempo entre mensagens que você gostaria (em segundos, somente números)? Para desabilitar, basta digitar 0 (zero).";
      bot.sendMessage(bot.message[i][4], message_timer, "");
      timer_d =1;
    }
    
    
    if (bot.message[i][5] == "\\/start") {
      String wellcome = "Olá! Eu sou o SmarttESP Bot! Eu estou rodando em um microcontrolador ESP8266. Isso é o que eu sei fazer até o momento:";
      String wellcome1 = "/voltage : lê o valor do ADC e publica no Telegram";
      String wellcome2 = "/ip : envia pelo Telegram o meu IP";
      String wellcome3 = "/send voltage by mqtt: publica a medida de ADC no tópico MQTT";
      String wellcome4 = "/send GPIO 4 by mqtt: publica o estado no tópico MQTT";
      String wellcome5 = "/send GPIO 5 by mqtt: publica o estado no tópico MQTT";
      String wellcome6 = "/config: configura os parâmetros de conexão do ESP8266";
      String wellcome7 = "/timer : configura um timer para envio de dados via MQTT com uma frequência constante";
      bot.sendMessage(bot.message[i][4], wellcome, "");
      bot.sendMessage(bot.message[i][4], wellcome1, "");
      bot.sendMessage(bot.message[i][4], wellcome2, "");
      bot.sendMessage(bot.message[i][4], wellcome3, "");
      bot.sendMessage(bot.message[i][4], wellcome4, "");
      bot.sendMessage(bot.message[i][4], wellcome5, "");
      bot.sendMessage(bot.message[i][4], wellcome6, "");
      bot.sendMessage(bot.message[i][4], wellcome7, "");
     }
  
  if (bot.message[i][5] == "\\/config") {
      String menssagem_cfg = "Você entrou no modo de configuração. Nesse modo você tem acesso aos comandos /mqtt_server, /mqtt_port, /mqtt_login, /mqtt_password, /mqtt_in_channel, /mqtt_out_channel"; //, /gpio_config, /adcmode
      String menssagem_cfg2 = "Para salvar as configurações, use o comando /save. Se quiser sair do modo de configuração sem salvar os dados, basta digitar /no_save em qualquer momento.";
      bot.sendMessage(bot.message[i][4], menssagem_cfg, "");
      bot.sendMessage(bot.message[i][4], menssagem_cfg2, "");
      
      configure_d=1;
      }
      
      if (configure_d)
            {
                 if (bot.message[i][5] == "\\/no_save") 
                    {
                        String menssagem_nosave = "Restaurando valores anteriores e saindo do modo de configuração.";
                        strcpy(mqtt_server_temp, "");
                        strcpy(mqtt_port_temp, "");
                        strcpy(mqtt_login_temp, "");
                        strcpy(mqtt_pass_temp, "");
                        strcpy(mqtt_in_channel_temp, "");
                        strcpy(mqtt_out_channel_temp, "");
                        bot.sendMessage(bot.message[i][4], menssagem_nosave, "");
                        mqtt_server_d=0;
                        mqtt_port_d=0;
                        mqtt_login_d=0;
                        mqtt_password_d=0;
                        mqtt_in_channel_d=0;
                        mqtt_out_channel_d=0;
                        gpio_config_d=0;
                        configure_d=0;
                        //ESP.reset();
                        bot.message[0][0] = "";
                        break;
                    }

                    if (bot.message[i][5] == "\\/save_config") 
                    {
                        strcpy(adc_type_cfg, adc_type_cfg_temp);
                        String menssagem_save = "Salvando a configuração para a EEPROM e reiniciando o ESP8266.";
                        saveConfigtoFile(mqtt_login_temp, device_id, mqtt_server_temp, mqtt_port_temp, mqtt_login_temp, mqtt_pass_temp, mqtt_in_channel_temp, mqtt_out_channel_temp, mqtt_channel_cmd_in, mqtt_channel_cmd_out);
                        bot.sendMessage(bot.message[i][4], menssagem_save, "");
                        delay(500);
                        ESP.reset();
                    }
                if (mqtt_server_d) 
                    {
                        //char *message_array;
                        String serverD = bot.message[i][5];
                        serverD.toCharArray(mqtt_server_temp,serverD.length()+1);
                        //strcpy(mqtt_server, message_array);
                        String menssagem_serverD = "Novo endereço do Broker MQTT recebido: ";
                        menssagem_serverD+= String(mqtt_server_temp);
                        bot.sendMessage(bot.message[i][4], menssagem_serverD, "");
                        mqtt_server_d =0;
                    }
                    
                if (mqtt_port_d) 
                    {
                        String serverPrd = bot.message[i][5];
                        serverPrd.toCharArray(mqtt_port_temp,serverPrd.length()+1);
                        String menssagemPrd = "Nova porta do Broker MQTT recebida: ";
                        menssagemPrd+= String(mqtt_port_temp);
                        bot.sendMessage(bot.message[i][4], menssagemPrd, "");
                        mqtt_port_d =0;
                    }
 
                if (mqtt_login_d) 
                    {
                        String serverLd = bot.message[i][5];
                        serverLd.toCharArray(mqtt_login_temp,serverLd.length()+1);
                        String menssagemLd = "Novo Login do Broker MQTT recebido: ";
                        menssagemLd+= String(mqtt_login_temp);
                        bot.sendMessage(bot.message[i][4], menssagemLd, "");
                        mqtt_login_d =0;
                    }

 
                if (mqtt_password_d) 
                    {
                        String serverPd = bot.message[i][5];
                        serverPd.toCharArray(mqtt_pass_temp,serverPd.length()+1);
                        String menssagemPd = "Novo senha do Broker MQTT recebido: ";
                        menssagemPd+= String(mqtt_pass_temp);
                        bot.sendMessage(bot.message[i][4], menssagemPd, "");
                        mqtt_password_d =0;
                    }

                if (mqtt_in_channel_d) 
                    {
                        String serverICd = bot.message[i][5];
                        serverICd.toCharArray(mqtt_in_channel_temp,serverICd.length()+1);
                        String menssagemICd = "Novo canal de entrada MQTT recebido: ";
                        menssagemICd+= String(mqtt_in_channel_temp);
                        bot.sendMessage(bot.message[i][4], menssagemICd, "");
                        mqtt_in_channel_d =0;
                    }
                if (mqtt_out_channel_d) 
                    {
                        String serverOCd = bot.message[i][5];
                        serverOCd.toCharArray(mqtt_out_channel_temp,serverOCd.length()+1);
                        String menssagemOCd = "Novo canal de saída MQTT recebido: ";
                        menssagemOCd+= String(mqtt_out_channel_temp);
                        bot.sendMessage(bot.message[i][4], menssagemOCd, "");
                        mqtt_out_channel_d =0;
                    }
                    
               if (adc_mode_d) 
                    {
                        String serverADCd = bot.message[i][5];
                        if (serverADCd == "externo") 
                          {
                            adc_type_cfg_temp[0] = 'e';
                            adc_type_cfg_temp[1] = 'x';
                            adc_type_cfg_temp[2] = 't';
                          }
                        else if (serverADCd == "VCC") 
                          {
                            adc_type_cfg_temp[0] = 'v';
                            adc_type_cfg_temp[1] = 'c';
                            adc_type_cfg_temp[2] = 'c';
                          }
                        else 
                          {
                            String menssagemADCd = "Não entendi o modo de funcionamento do ADC. Por favor escolha entre externo e VCC.";
                            bot.sendMessage(bot.message[i][4], menssagemADCd, "");
                            break;
                          }
                        String menssagemADCd2 = "Nova configuração de ADC recebida: ";
                        menssagemADCd2+= serverADCd;
                        bot.sendMessage(bot.message[i][4], menssagemADCd2, "");
                        mqtt_out_channel_d =0;
                    }

                if (bot.message[i][5] == "\\/mqtt_server") 
                    {
                        String menssagemSv = "Qual o endereço do Broker MQTT (endereço atual: ";
                        menssagemSv+= String(mqtt_server);
                        menssagemSv+= ")?";
                        bot.sendMessage(bot.message[i][4], menssagemSv, "");
                        mqtt_server_d=1;
                    }
                    
               if (bot.message[i][5] == "\\/mqtt_port") 
                    {
                        String menssagemPr = "Qual a porta do Broker MQTT (porta atual: ";
                        menssagemPr+= String(mqtt_port);
                        menssagemPr+= ")?";
                        bot.sendMessage(bot.message[i][4], menssagemPr, "");
                        mqtt_port_d=1;
                    }
                    
               if (bot.message[i][5] == "\\/mqtt_login") 
                    {
                        String menssagemL = "Qual o Login do Broker MQTT (login atual: ";
                        menssagemL+= String(mqtt_login);
                        menssagemL+= ")?";
                        bot.sendMessage(bot.message[i][4], menssagemL, "");
                        mqtt_login_d=1;
                    }
               if (bot.message[i][5] == "\\/mqtt_password") 
                    {
                        String menssagemP = "Qual a senha do Broker MQTT (senha atual: ";
                        menssagemP+= String(mqtt_pass);
                        menssagemP+= ")?";
                        bot.sendMessage(bot.message[i][4], menssagemP, "");
                        mqtt_password_d=1;
                    }
              if (bot.message[i][5] == "\\/mqtt_in_channel") 
                    {
                        String menssagemIC = "Qual o canal de subscrição MQTT (canal atual: ";
                        menssagemIC+= String(mqtt_channel_in);
                        menssagemIC+= ")?";
                        bot.sendMessage(bot.message[i][4], menssagemIC, "");
                        mqtt_in_channel_d=1;
                    }
              if (bot.message[i][5] == "\\/mqtt_out_channel") 
                    {
                        String menssagemOC = "Qual o canal de publicação MQTT (canal atual: ";
                        menssagemOC+= String(mqtt_channel_out);
                        menssagemOC+= ")?";
                        bot.sendMessage(bot.message[i][4], menssagemOC, "");
                        mqtt_out_channel_d=1;
                    }

              if (bot.message[i][5] == "\\/adc_mode") 
                    {
                        String menssagemA = "Você gostaria de configurar o ADC, como externo ou VCC? Atualmente o ADC está configurado como";
                        if (adc_type_cfg=="vcc") menssagemA+= String(" VCC.");
                        else menssagemA+= String(" externo.");
                        bot.sendMessage(bot.message[i][4], menssagemA, "");
                        adc_mode_d=1;
                    }
                    
             if (bot.message[i][5] == "\\/gpio_config") 
                    {
                        String menssagemG = "Nessa versão do Firmware temos os GPIOs 4 e 5 como input e os GPIOs 12, 13 e 14 como output.";
                        //String menssagem = "Vamos configurar agora os GPIOs 4, 5, 12, 13 e 14.";
                        //String menssagem2 = "Como você gostaria de configurar o GPIO 4 (opções: INPUT, OUTPUT ou INPUT_PULLUP).";
                        bot.sendMessage(bot.message[i][4], menssagemG, "");
                        //bot.sendMessage(bot.message[i][4], menssagem2, "");
                        //gpio_config_d=1;
                    }

               
            }
    
  
      
    }
  }
  bot.message[0][0] = "";   // All messages have been replied - reset new messages
}


  
 // Loop com o programa principal
void loop(){
 delay(100);
 
// Se desconectado, reconectar.
 if (!client.connected()) {
                             disconnected = 1;
                             reconnectcount = reconnect(client, api_key, mqtt_login, mqtt_pass);
                             if (reconnectcount<10) 
                                  {
                                    client.subscribe(mqtt_topic_in);
                                    client.subscribe(mqtt_topic_cmd_in);
                                  }
                           }
  if(change_timer)
          {
            timer.detach();
            delay(200);
            if (config_period_I>0) timer.attach_ms(1000*config_period_I,TimerMaker);
            change_timer=0;
          }
          
   if (marker_work) 
   {
    sendMQTTdata(api_key, client);
    marker_work =0;
   }
   
  client.loop();
  delay(10);  
  if (millis() > Bot_lasttime + Bot_mtbs)  
    {
      //bot.getUpdates(bot.message[0][1]);   // launch API GetUpdates up to xxx message
      //Bot_ExecMessages();   // reply to message with Echo
      Bot_lasttime = millis();
    }  
 

}

