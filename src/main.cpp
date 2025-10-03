/*
  Ideias Maker - Marcos Rocha - 2021
  Se inscreva: https://www.youtube.com/channel/UCtj5BL5VB0JtueLTQwzDjFQ/featured
  Start Stop Bluetooth por presença e aplicativo.

  *BIBLIOTECAS ESTÃO NA PASTA ANTERIOR*

  STATUS DO ALARME:
  0  - Fechado
  1  - Aberto
  3  - Acionado 
  10 - Desativado

  Pontos importantes para alterar:
    * Códigos controle RF (Inserir o código do controle que irá usar)    -- Linha 46
    * Token Blynk - Inserir o Token conforme aplicativo                  -- Linha 71
    * Encontrar dispositivo Bluetooth                                    -- Linha 318
    * MAC Address Bluetooth - Inserir o MAC do dispositivo que irá usar) -- Linha 91
    * TARGET_RSSI - Distancia para desbloqueio do carro                  -- Linha 92
    * Intervalo de busca do Bluetooth                                    -- Linha 512
*/
#define BLYNK_TEMPLATE_ID "123"
#define BLYNK_TEMPLATE_NAME "123"

#include <BlynkSimpleEsp32_BT.h>
#include <RF_HT6P20.h>
#include <SimpleTimer.h> 


#define RELAY_ACC 32
#define RELAY_IG1 33
#define RELAY_IG2 25
#define RELAY_MOTOR_ARRANQUE 26
#define RELAY_ALARME_GPS 56     //PROXIMO VIDEO - RASTREADOR

#define RELAY_TRAVA 13
#define RELAY_DESTRAVA 12
#define RELAY_BUZINA 27
#define RELAY_PISCA_ALERTA 14

#define BOTAO_START_STOP 15
#define FREIO 17
#define PORTA_ABERTA 16

int STATUS_BOTAO_START_STOP = 0;
int STATUS_CARRO = 0;
int STATUS_FREIO = 0;
int CONTROLA_TRAVA_FREIO = 0;
int STATUS_PORTA = 0;
int STATUS_PORTA_ABERTA = 0;
int STATUS_ALARME = 0;
int STATUS_PARTIDA_REMOTA = 0;
//

//CONTROLES //INFORMAR O CÓDIGO DO CONTROLE NO PADRÃO "0x5555555"
#define controle01_TRAVA 0x5555555
#define controle01_DESTRAVA 0x5555555
#define controle01_PARTIDA_REMOTA 0x5555555

#define controle02_TRAVA 0x5555555
#define controle02_DESTRAVA 0x5555555
#define controle02_PARTIDA_REMOTA 0x5555555
#define controle02_ABERTURA_VIDROS 0x5555555

#define controle03_TRAVA 0x5555555
#define controle03_DESTRAVA 0x5555555
#define controle03_PARTIDA_REMOTA 0x5555555
#define controle03_ABERTURA_VIDROS 0x5555555

int destravadoControle= 0;
int controleMillsAplicativo = 0;
int contrMillisTrava = 0;
int contrMillisBluetooth = 0;
//

//BLUETOOTH APLICATIVO
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
char auth[] = "INSIRA SEU TOKEN AQUI"; //APLICATIVO BLYNK
WidgetTerminal terminal(V4);
WidgetLED ledAberto(V5);
WidgetLED ledFechado(V6);
WidgetLED ledMotor(V7);
//

//RADIO FREQUENCIA
RF_HT6P20 RF;
int pinRF_RX = 22;
//

// TIMER
SimpleTimer timer;
//

//BLUETOOTH POR PRESENÇA (LOW ENERGY)
#include <BLEDevice.h>
#define ADDRESS "INFORME O MAC ADDRESS DO SEU BLUETOOTH" 
#define TARGET_RSSI -70 //DISTANCIA PARA DESBLOQUEIO POR PRESENÇA
#define MAX_MISSING_TIME 21000 //TEMPO PARA BLOQUEIO
BLEScan* pBLEScan; 
boolean found = false; 
uint32_t lastFoundTime = 0; 
int rssi = 0;
int controleBluetooth = 0;
//

//FUNÇÕES CARRO
void ligaCarro();
void desligaCarro();
void ligaAcc();
void ligaPosChave();
void desligaTudo();
//

//FUNCÕES ALARME
void destravaCarro();
void travaCarro();
void encontrarCarro();
void abrirVidros();
//

void ligaCarro(){
  //LIGA O CARRO
  terminal.clear();
  terminal.println("----------------------------------------------------");
  terminal.println("I D E I A S   M A K E R       -                 2021");
  terminal.println("----------------------------------------------------");
  terminal.println("");
  terminal.println("");  
  terminal.println("");

  digitalWrite(RELAY_ACC, LOW);
  delay(1000);
  digitalWrite(RELAY_IG1, LOW);
  digitalWrite(RELAY_IG2, LOW);
  delay(3000);
  digitalWrite(RELAY_MOTOR_ARRANQUE, LOW);
  delay(700);    
  digitalWrite(RELAY_MOTOR_ARRANQUE, HIGH);  
  Serial.println("OK");
  terminal.println("                C A R R O - L I G A D O           ");
  ledMotor.on();
  terminal.flush();
  STATUS_PARTIDA_REMOTA = 1;
  STATUS_BOTAO_START_STOP = 3;
  STATUS_CARRO = 1;
  delay(1000);  
}

void desligaCarro(){
  //DESLIGA O CARRO
  terminal.clear();
  terminal.println("----------------------------------------------------");
  terminal.println("I D E I A S   M A K E R       -                 2021");
  terminal.println("----------------------------------------------------");
  terminal.println("");
  terminal.println("");
  terminal.println("");
  terminal.println("");
  terminal.println("            C A R R O - D E S L I G A D O           ");
  ledMotor.off();
  terminal.flush();
  //Desliga ACC e Ignição
  digitalWrite(RELAY_ACC, HIGH);
  digitalWrite(RELAY_IG1, HIGH);
  digitalWrite(RELAY_IG2, HIGH);
  STATUS_BOTAO_START_STOP = 0;
  STATUS_PARTIDA_REMOTA = 0;
  STATUS_CARRO = 0;
  //DESLIGA TODOS OS RELÉS
  delay(3000);
  Serial.println("OK");
  CONTROLA_TRAVA_FREIO = 0;
}

void ligaAcc(){
  //LIGA O ACC
  digitalWrite(RELAY_ACC, LOW);
  Serial.println("ACC LIGADO");
  STATUS_BOTAO_START_STOP = 1;
  delay(300);
}

void ligaPosChave(){
  //LIGA O PÓS CHAVE E BOMBA DE COMBUSTIVEL
  digitalWrite(RELAY_IG1, LOW);
  digitalWrite(RELAY_IG2, LOW);
  Serial.println("PÓS CHAVE LIGADO E BOMBA DE COMBUSTIVEL");
  STATUS_BOTAO_START_STOP = 2;
  delay(300);
}

void desligaTudo(){
  //DESLIGA TUDO
  digitalWrite(RELAY_ACC, HIGH);
  digitalWrite(RELAY_IG1, HIGH);
  digitalWrite(RELAY_IG2, HIGH);
  Serial.println("DESLIGA TUDO");
  STATUS_BOTAO_START_STOP = 0;
  STATUS_PARTIDA_REMOTA = 0;
  STATUS_CARRO = 0;
  ledMotor.off();
  delay(300); 
}

void destravaCarro(){
  //PORTAS ABERTAS
  //PRIMEIRO DESLIGA A BUZINA
  digitalWrite(RELAY_BUZINA, HIGH); 
  terminal.clear();
  terminal.println("----------------------------------------------------");
  terminal.println("I D E I A S   M A K E R       -                 2021");
  terminal.println("----------------------------------------------------");
  terminal.println("");
  terminal.println("");
  terminal.println("");
  terminal.println("");
  terminal.println("              C A R R O - A B E R T O      ");
  ledFechado.off();
  ledAberto.on();
  terminal.flush();
  delay(200);
  digitalWrite(RELAY_PISCA_ALERTA, LOW);
  digitalWrite(RELAY_DESTRAVA, LOW);
  delay(300);
  digitalWrite(RELAY_PISCA_ALERTA, HIGH);
  digitalWrite(RELAY_DESTRAVA, HIGH);
  delay(300);
  digitalWrite(RELAY_PISCA_ALERTA, LOW);
  delay(300);
  digitalWrite(RELAY_PISCA_ALERTA, HIGH);     
  controleBluetooth = 1;
  STATUS_ALARME = 1;
}

void travaCarro(){
  if(STATUS_PORTA_ABERTA == 1){
    digitalWrite(RELAY_BUZINA,LOW);  delay(10);
    digitalWrite(RELAY_BUZINA, HIGH);delay(10);
    digitalWrite(RELAY_PISCA_ALERTA, LOW); delay(300);
    digitalWrite(RELAY_PISCA_ALERTA, HIGH);delay(300);
    digitalWrite(RELAY_PISCA_ALERTA, LOW); delay(300);
    digitalWrite(RELAY_PISCA_ALERTA, HIGH);delay(300);  
    digitalWrite(RELAY_PISCA_ALERTA, LOW); delay(300);
    digitalWrite(RELAY_PISCA_ALERTA, HIGH);delay(300); 
    digitalWrite(RELAY_PISCA_ALERTA, LOW); delay(300);
    digitalWrite(RELAY_PISCA_ALERTA, HIGH);delay(200);
    Serial.println("Alguma porta está aberta!");
  }
  else{
    //PORTAS FECHADAS
    //PRIMEIRO DESLIGA A BUZINA
    digitalWrite(RELAY_BUZINA, HIGH); 
    terminal.clear();
    terminal.println("----------------------------------------------------");
    terminal.println("I D E I A S   M A K E R       -                 2021");
    terminal.println("----------------------------------------------------");
    terminal.println("");
    terminal.println("");
    terminal.println("");
    terminal.println("");
    terminal.println("              C A R R O - F E C H A D O       ");
    ledAberto.off();
    ledFechado.on();
    terminal.flush();
    digitalWrite(RELAY_PISCA_ALERTA, LOW);
    digitalWrite(RELAY_TRAVA, LOW);
    delay(300);
    digitalWrite(RELAY_PISCA_ALERTA, HIGH);  
    digitalWrite(RELAY_TRAVA, HIGH);  
    digitalWrite(RELAY_BUZINA, HIGH);  
    controleBluetooth = 0;
    STATUS_ALARME = 0;
    STATUS_PORTA = 0;
    desligaTudo();
    CONTROLA_TRAVA_FREIO = 0;
    controleBluetooth = 0;
  }
}

void encontrarCarro(){
  digitalWrite(RELAY_BUZINA,LOW);  delay(50);
  digitalWrite(RELAY_BUZINA, HIGH);delay(50);
  digitalWrite(RELAY_BUZINA, LOW); delay(50);
  digitalWrite(RELAY_BUZINA, HIGH);  
  digitalWrite(RELAY_PISCA_ALERTA, LOW); delay(300);
  digitalWrite(RELAY_PISCA_ALERTA, HIGH);delay(300);
  digitalWrite(RELAY_PISCA_ALERTA, LOW); delay(300);
  digitalWrite(RELAY_PISCA_ALERTA, HIGH);delay(300);  
  digitalWrite(RELAY_PISCA_ALERTA, LOW); delay(300);
  digitalWrite(RELAY_PISCA_ALERTA, HIGH);  
  Serial.println("OK");
}

void abrirVidros(){
  digitalWrite(RELAY_BUZINA, HIGH); 
  terminal.clear();
  terminal.println("----------------------------------------------------");
  terminal.println("I D E I A S   M A K E R       -                 2021");
  terminal.println("----------------------------------------------------");
  terminal.println("");
  terminal.println("");
  terminal.println("");
  terminal.println("");
  terminal.println("              V I D R O S - A B E R T O S      ");
  ledFechado.off();
  ledAberto.on();
  terminal.flush();
  Serial.println("OK");  
  digitalWrite(RELAY_PISCA_ALERTA, LOW); digitalWrite(RELAY_DESTRAVA, LOW); delay(500);
  digitalWrite(RELAY_PISCA_ALERTA, HIGH); digitalWrite(RELAY_DESTRAVA, HIGH);  delay(500);
  digitalWrite(RELAY_PISCA_ALERTA, LOW); digitalWrite(RELAY_DESTRAVA, LOW); delay(500);
  digitalWrite(RELAY_PISCA_ALERTA, HIGH); digitalWrite(RELAY_DESTRAVA, HIGH);  delay(500);
  digitalWrite(RELAY_PISCA_ALERTA, LOW); digitalWrite(RELAY_DESTRAVA, LOW); delay(500);
  digitalWrite(RELAY_PISCA_ALERTA, HIGH); digitalWrite(RELAY_DESTRAVA, HIGH);  delay(500);
  controleBluetooth = 1;
  STATUS_ALARME = 1;
  destravadoControle = 1;  
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks{  
    void onResult(BLEAdvertisedDevice advertisedDevice){
        //Serial.println("Dispositivo encontrado: ");      
        //Serial.println(advertisedDevice.toString().c_str());
        if(advertisedDevice.getAddress().toString() == ADDRESS){
            rssi = advertisedDevice.getRSSI();
            Serial.println("RSSI: ");
            Serial.println(rssi);
              
            if(rssi > TARGET_RSSI){
              lastFoundTime = millis();
              found = true;
              advertisedDevice.getScan()->stop();
              controleMillsAplicativo = 1;
            }
            else{
              found = false;
            }
        }
        else{
          found = false;
        }      
    }
};

void CheckConnectionBluetooth(){
  found = false;
  pBLEScan->start(1);
}

BLYNK_WRITE(V0){
  int parametro = param.asInt();
  if (parametro == 0){
    if(STATUS_ALARME == 1 || STATUS_ALARME == 3){
      travaCarro();
      Serial.println("");
      Serial.println("Aplicativo - Travando o carro!");
    }
  }
}

BLYNK_WRITE(V1){
  int parametro = param.asInt();
  if (parametro == 0){
    if(STATUS_ALARME == 0 || STATUS_ALARME == 3){
      Serial.println("");
      Serial.println("Aplicativo - Destravando o carro!");
      destravaCarro();
      destravadoControle = 1;
      controleMillsAplicativo = 1;
    }
  }
}

BLYNK_WRITE(V2){
  int parametro = param.asInt();
  if (parametro == 0){
    if(STATUS_CARRO == 0 && STATUS_PARTIDA_REMOTA == 0 && STATUS_ALARME != 10){
      Serial.println("");
      Serial.println("Aplicativo - Ligando o carro!");
      Serial.println("");
      delay(2000);
      ligaCarro();
      STATUS_PARTIDA_REMOTA = 1;
    } 
  }
}

BLYNK_WRITE(V3){
  int parametro = param.asInt();
  if (parametro == 0){
    if(STATUS_CARRO == 1 && STATUS_PARTIDA_REMOTA == 1){
      Serial.println("");
      Serial.println("Aplicativo - Desligando o carro!");
      Serial.println("");
      delay(2000);
      desligaCarro();
      STATUS_PARTIDA_REMOTA = 0;
      controleMillsAplicativo = 1;
    }
  }
}

BLYNK_WRITE(V4){
  if (String("DESATIVAR ALARME") == param.asStr()){
    destravaCarro();
    terminal.clear();
    terminal.println("----------------------------------------------------");
    terminal.println("I D E I A S   M A K E R       -                 2021");
    terminal.println("----------------------------------------------------");
    terminal.println("");
    terminal.println("");
    terminal.println("");
    terminal.println("");
    terminal.println("Alarme desativado permanente até o reinício do ESP");    
    Serial.println("Alarme desativado permanente até o reinício do ESP"); 
    STATUS_ALARME = 10;
  } 
  else if (String("ATIVAR ALARME") == param.asStr()){
    travaCarro();
    terminal.clear();
    terminal.println("----------------------------------------------------");
    terminal.println("I D E I A S   M A K E R       -                 2021");
    terminal.println("----------------------------------------------------");
    terminal.println("");
    terminal.println("");
    terminal.println("");
    terminal.println("");
    terminal.println("Alarme alarme ativado novamente!");    
    Serial.println("Alarme alarme ativado novamente!");  
  }

  // Ensure everything is sent
  terminal.flush();
}

BLYNK_WRITE(V5){
  int parametro = param.asInt();
  if (parametro == 0){
      Serial.println("Aplicativo - Funcão encontrar carro!");
      encontrarCarro();
  }
}

BLYNK_WRITE(V6){
  int parametro = param.asInt();
  if (parametro == 0){
      Serial.println("Aplicativo - Funcão abrir os vidros!");
      abrirVidros();
      controleMillsAplicativo = 1;
  }
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
    terminal.clear();
    terminal.println("----------------------------------------------------");
    terminal.println("I D E I A S   M A K E R       -                 2021");
    terminal.println("----------------------------------------------------");
    terminal.println("");
    terminal.println("");
    terminal.println("Bluetooth conectado!");
    terminal.println("");
    terminal.println("Seja bem vindo Marcos Rocha!");  
    terminal.flush();
    if(STATUS_CARRO == 0){
        ledMotor.off();
    }
    else{
      ledMotor.on();
    }
    if(STATUS_ALARME == 0){
      ledAberto.off();
      ledFechado.on();
    }
    else{
      ledFechado.off();
      ledAberto.on();
    }
}

void setup(){
    RF.beginRX(pinRF_RX);    
    Serial.begin(9600);
    Blynk.setDeviceName("Ideias Maker - 2021");  
    Blynk.begin(auth);

    pinMode(RELAY_ACC, OUTPUT);
    pinMode(RELAY_IG1, OUTPUT);
    pinMode(RELAY_IG2, OUTPUT);
    pinMode(RELAY_MOTOR_ARRANQUE, OUTPUT);

    pinMode(RELAY_TRAVA, OUTPUT);
    pinMode(RELAY_DESTRAVA, OUTPUT);
    pinMode(RELAY_BUZINA, OUTPUT);
    pinMode(RELAY_PISCA_ALERTA, OUTPUT);
    pinMode(RELAY_ALARME_GPS, OUTPUT);

    pinMode(BOTAO_START_STOP, INPUT);
    pinMode(FREIO, INPUT);
    pinMode(PORTA_ABERTA, INPUT);
     
    digitalWrite(RELAY_ACC, HIGH);
    digitalWrite(RELAY_IG1, HIGH);
    digitalWrite(RELAY_IG2, HIGH);
    digitalWrite(RELAY_MOTOR_ARRANQUE, HIGH);

    digitalWrite(RELAY_TRAVA, HIGH);
    digitalWrite(RELAY_DESTRAVA, HIGH);
    digitalWrite(RELAY_BUZINA, HIGH);
    digitalWrite(RELAY_PISCA_ALERTA, HIGH);
    digitalWrite(RELAY_ALARME_GPS, HIGH);
    
    BLEDevice::init(""); 
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    timer.setInterval(7000L, CheckConnectionBluetooth); //INTERVALO DE BUSCA BLUETOOTH
}

void loop(){ 
    static unsigned long delayAlarmeAcionado = millis();
    static unsigned long delayTravaControle = millis(); 
    static unsigned long delayPortaAberta = millis(); 
    static unsigned long delayTravaPortaFreio = millis(); 
    uint32_t now = millis(); 
    timer.run();  
    Blynk.run();    
    byte VALOR_FREIO = digitalRead(FREIO);
    byte VALOR_BOTAO_START_STOP = digitalRead(BOTAO_START_STOP);
    byte VALOR_PORTA_ABERTA = digitalRead(PORTA_ABERTA);
    
    //SISTEMA DE ALARME
    if (STATUS_ALARME == 3) {    
      
      if((millis() - delayAlarmeAcionado) < 400){
        digitalWrite(RELAY_PISCA_ALERTA, HIGH);
        digitalWrite(RELAY_BUZINA, HIGH);
      }
      else{
        digitalWrite(RELAY_PISCA_ALERTA, LOW);
        digitalWrite(RELAY_BUZINA, LOW);
      }
      if((millis() - delayAlarmeAcionado) > 800){
        delayAlarmeAcionado = millis();
      }
    }
    //

    //PORTA
    if(VALOR_PORTA_ABERTA){
        delayTravaPortaFreio = millis(); 
        contrMillisTrava = 0;
        
        if (STATUS_ALARME == 0){
          Serial.println("ALARME ACIONADO! PORTA ABERTA COM O ALARME BLOQUEADO!");
          terminal.clear();
          terminal.println("ALARME ACIONADO! PORTA ABERTA COM O ALARME BLOQUEADO");
          terminal.flush();
          //MUDA O STATUS PARA ALARME ACIONADO - EM CASO DE ABRIR A PORTA COM O ALARME BLOQUEADO.
          STATUS_ALARME = 3;
          //AVISA O OUTRO ARDUINO GPS/BOMBA DE COMBUSTIVEL
          digitalWrite(RELAY_ALARME_GPS, LOW);
          delay(300);
          digitalWrite(RELAY_ALARME_GPS, HIGH);
          Serial.println("");
          Serial.println("Acionado o ALARME GPS");
          Serial.println("");
          //DESLIGA TUDO
          desligaTudo();
        }
        if (STATUS_ALARME == 1){
          //Serial.println("");
          //Serial.println("PORTA ABERTA!");
          //Serial.println("");
          STATUS_PORTA = 1;   
          STATUS_PORTA_ABERTA = 1;   
          delayPortaAberta = millis();   
          lastFoundTime = millis();
          now = millis();
        }
    }
    else STATUS_PORTA_ABERTA = 0;
    //

    //FREIO
    if(VALOR_FREIO){

        if(STATUS_PARTIDA_REMOTA == 1 && CONTROLA_TRAVA_FREIO == 0 && STATUS_ALARME == 1){
          digitalWrite(RELAY_TRAVA, LOW);
          delay(100);
          digitalWrite(RELAY_TRAVA, HIGH);  
          CONTROLA_TRAVA_FREIO = 1;          
        }
      
        STATUS_FREIO = 1;   
        if (STATUS_ALARME == 0){
          Serial.println("ALARME ACIONADO! FREIO PRESSIONADO COM O ALARME BLOQUEADO!");
          terminal.clear();
          terminal.println("ALARME ACIONADO! FREIO PRESSIONADO COM O ALARME BLOQUEADO!");
          terminal.flush();
          //MUDA O STATUS PARA ALARME ACIONADO - EM CASO DE PRESSIONAR O FREIO COM O ALARME BLOQUEADO.
          STATUS_ALARME = 3;
          //AVISA O OUTRO ARDUINO GPS/BOMBA DE COMBUSTIVEL
          digitalWrite(RELAY_ALARME_GPS, LOW);
          delay(300);
          digitalWrite(RELAY_ALARME_GPS, HIGH);
          Serial.println("");
          //DESLIGA TUDO
          desligaTudo();
        }
        //if (STATUS_ALARME == 1){
          //Serial.println("");
          //Serial.println("FREIO PRESSIONADO!");
          //Serial.println("");
        //}
    }
    else{
        STATUS_FREIO = 0; 
    }
    //

    // BOTÃO START STOP
    if(VALOR_BOTAO_START_STOP){
      if (STATUS_ALARME == 0){
            Serial.println("ALARME ACIONADO! BOTÃO PRESSIONADO COM O ALARME BLOQUEADO!");
            terminal.clear();
            terminal.println("ALARME ACIONADO! BOTÃO PRESSIONADO COM O ALARME BLOQUEADO!");
            terminal.flush();
            //MUDA O STATUS PARA ALARME ACIONADO - EM CASO DE PRESSIONAR O BOTÃO COM O ALARME BLOQUEADO.
            STATUS_ALARME = 3;
            //AVISA O OUTRO ARDUINO GPS/BOMBA DE COMBUSTIVEL
            digitalWrite(RELAY_ALARME_GPS, LOW);
            delay(300);
            digitalWrite(RELAY_ALARME_GPS, HIGH);
            Serial.println("");
            //DESLIGA TUDO
            desligaTudo();
          }
      if (STATUS_FREIO == 1 && STATUS_CARRO == 0 && STATUS_ALARME == 1){
        Serial.println("Botão - Ligando o carro!");
        ligaCarro();
      }
      else if (STATUS_FREIO == 1 && STATUS_CARRO == 1 && STATUS_ALARME == 1){
        Serial.println("Botão - Desligando o carro!");
        digitalWrite(RELAY_DESTRAVA, LOW);
        delay(100);
        digitalWrite(RELAY_DESTRAVA, HIGH);
        desligaCarro();
        delayTravaControle = millis(); 
        delayPortaAberta = millis();
        lastFoundTime = millis();
        now = millis();
      }      
      else if (STATUS_BOTAO_START_STOP == 0 && STATUS_CARRO == 0 && STATUS_ALARME == 1){
        ligaAcc();
      }
      else if (STATUS_BOTAO_START_STOP == 1 && STATUS_CARRO == 0 && STATUS_ALARME == 1){
        ligaPosChave();
      }
      else if (STATUS_BOTAO_START_STOP == 2 && STATUS_FREIO == 0 && STATUS_CARRO == 0
               && STATUS_ALARME == 1){
        desligaTudo();
        delayTravaControle = millis(); 
        delayPortaAberta = millis();
        lastFoundTime = millis();
        now = millis();
      }
    }
    //

    //DESBLOQUEIO/BLOQUEIO/PARTIDA REMOTA POR CONTROLE  
    if(RF.available()){
      RF.getCode();
      Serial.print("RF Encontrado:  "); Serial.println(RF.getCode(),HEX);
    }
    
    //FUNCAO ENCONTRAR CARRO, 
    //AO APERTAR O TRAVAR COM O ALARME TRAVADO.
    if(
        RF.getCode() == controle01_TRAVA && STATUS_ALARME == 0 || 
        RF.getCode() == controle02_TRAVA && STATUS_ALARME == 0 ||
        RF.getCode() == controle03_TRAVA && STATUS_ALARME == 0){
          Serial.println("ControleRF - Funcão encontrar carro!");
          encontrarCarro();
    }
     //
      
    if(
        RF.getCode() == controle01_TRAVA && STATUS_ALARME == 1 || RF.getCode() == controle01_TRAVA && STATUS_ALARME == 3 ||
        RF.getCode() == controle02_TRAVA && STATUS_ALARME == 1 || RF.getCode() == controle02_TRAVA && STATUS_ALARME == 3 ||
        RF.getCode() == controle03_TRAVA && STATUS_ALARME == 1 || RF.getCode() == controle03_TRAVA && STATUS_ALARME == 3 
        ){          
          Serial.println("ControleRF - Travando o carro!");
          travaCarro();
    }
    if(
        RF.getCode() == controle01_DESTRAVA && STATUS_ALARME == 0 || RF.getCode() == controle01_DESTRAVA && STATUS_ALARME == 3 ||
        RF.getCode() == controle02_DESTRAVA && STATUS_ALARME == 0 || RF.getCode() == controle02_DESTRAVA && STATUS_ALARME == 3 ||
        RF.getCode() == controle03_DESTRAVA  && STATUS_ALARME == 0 || RF.getCode() == controle03_DESTRAVA  && STATUS_ALARME == 3
        ){
          Serial.println("ControleRF - Destravando o carro!");
          destravaCarro();
          destravadoControle = 1;
          delayTravaControle = millis(); 
    }  
    if(
        RF.getCode() == controle01_PARTIDA_REMOTA && STATUS_CARRO == 0 && STATUS_PARTIDA_REMOTA == 0 && STATUS_ALARME != 10 ||
        RF.getCode() == controle02_PARTIDA_REMOTA && STATUS_CARRO == 0 && STATUS_PARTIDA_REMOTA == 0 && STATUS_ALARME != 10 ||
        RF.getCode() == controle03_PARTIDA_REMOTA  && STATUS_CARRO == 0 && STATUS_PARTIDA_REMOTA == 0 && STATUS_ALARME != 10 
        ){
          Serial.println("ControleRF - Ligando o carro!");
          delay(2000);
          ligaCarro();
          STATUS_PARTIDA_REMOTA = 1;
    }
    else if(
        RF.getCode() == controle01_PARTIDA_REMOTA && STATUS_CARRO == 1 && STATUS_PARTIDA_REMOTA == 1 ||
        RF.getCode() == controle02_PARTIDA_REMOTA && STATUS_CARRO == 1 && STATUS_PARTIDA_REMOTA == 1 ||
        RF.getCode() == controle03_PARTIDA_REMOTA && STATUS_CARRO == 1 && STATUS_PARTIDA_REMOTA == 1
        ){
          Serial.println("ControleRF - Desligando o carro!");
          delay(2000);
          desligaCarro();
          STATUS_PARTIDA_REMOTA = 0;
          delayTravaControle = millis();
          delayPortaAberta = millis();
    }
    if(
        RF.getCode() == controle02_ABERTURA_VIDROS && STATUS_ALARME == 0 || RF.getCode() == controle02_ABERTURA_VIDROS && STATUS_ALARME == 3 ||
        RF.getCode() == controle03_ABERTURA_VIDROS && STATUS_ALARME == 0 || RF.getCode() == controle03_ABERTURA_VIDROS && STATUS_ALARME == 3
        ){
          Serial.println("ControleRF - Abrindo os Vidros e Destravando o Carro!");
          abrirVidros(); 
          delayTravaControle = millis();          
    }  
    //

    //APLICATIVO - CONTROLA O TEMPO DE BLOQUEIO APÓS O DESBLOQUEIO (SEGURANÇA)
    if (controleMillsAplicativo == 1){
      controleMillsAplicativo = 0;
      delayTravaControle = millis(); 
      delayPortaAberta = millis();
    }
    //

    //SISTEMA - CONTROLA O TEMPO DE BLOQUEIO APÓS O DESBLOQUEIO (SEGURANÇA)
    if ((millis() - delayTravaControle) > 60000 /*01 MINUTO*/ && destravadoControle == 1 && STATUS_BOTAO_START_STOP == 0
    && STATUS_ALARME == 1 && STATUS_PORTA == 0){
       Serial.println("Sistema - Travando o carro!");
       travaCarro();
       destravadoControle = 0;   
    }
    //
    
    if(found && controleBluetooth == 0 && rssi > TARGET_RSSI){ 
      Serial.println("Presença Bluetooth - Destravando o carro!");
      destravaCarro();
      destravadoControle = 1;
      delayTravaControle = millis(); 
      contrMillisBluetooth = 1;
    }

    if (now - lastFoundTime > MAX_MISSING_TIME && 
        found == false && controleBluetooth == 1 && STATUS_CARRO == 0 && contrMillisBluetooth == 1 && STATUS_BOTAO_START_STOP == 0 && STATUS_PORTA_ABERTA == 0){
       Serial.println("Presença Bluetooth - Travando o carro!");
       travaCarro();
       destravadoControle = 0;  
       controleMillsAplicativo = 1;
       contrMillisBluetooth = 0;
     }
    
    //SISTEMA - CONTROLA O TEMPO DE BLOQUEIO APÓS O DESBLOQUEIO (SEGURANÇA)
    if ((millis() - delayPortaAberta) > 3600000 /*01 HORA*/ && STATUS_BOTAO_START_STOP == 0 && STATUS_ALARME == 1 && STATUS_PORTA == 1 && found == false){
       Serial.println("Sistema - Travando o carro! Após porta Aberta!");
       travaCarro();
       destravadoControle = 0;   
    }
    //    

    if ((millis() - delayTravaPortaFreio) > 30000 && contrMillisTrava == 0 && STATUS_PARTIDA_REMOTA == 1 && STATUS_ALARME == 1 && found == false){
       Serial.println("Sistema - Travando as portas após portas abertas!");
       CONTROLA_TRAVA_FREIO = 0; 
       contrMillisTrava = 1;
    }
}
