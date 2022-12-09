/* -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

 */

#include <LiquidCrystal.h>
#include <SPI.h> //INCLUSÃO DE BIBLIOTECA
#include <MFRC522.h> //INCLUSÃO DE BIBLIOTECA
#include <Servo.h> //INCLUSÃO DE BIBLIOTECA
#include <Ultrasonic.h>



#define SS_PIN 53 //PINO SDA
#define RST_PIN 5 //PINO DE RESET
#define pino_trigger 7
#define pino_echo 8


Ultrasonic ultrasonic(pino_trigger, pino_echo);

MFRC522 rfid(SS_PIN, RST_PIN); //PASSAGEM DE PARÂMETROS REFERENTE AOS PINOS

Servo s; //OBJETO DO TIPO SERVO
int pos; //POSIÇÃO DO SERVO

const int pinoServo = 3; // PINO DIGITAL REFERENTE AO SERVO MOTOR

byte anguloCancelaFechada = 6; //VALOR DO ÂNGULO PARA CANCELA FECHADA
byte anguloCancelaAberta = 90; //VALOR DO ÂNGULO PARA CANCELA ABERTA

//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(43, 41, 37, 35, 33, 31);
// PORTAS LCD =  (RS, E , D4, D5, D6, D7)


int pwm = 13; // pino controle do brilho
int creditotag1 = 50;
int creditotag2 = 50;
bool isCancelaOpen = false;
int led_verm = 11;
int led_verd = 10;

void setup() {
  Serial.begin(9600); //INICIALIZA A SERIAL
  SPI.begin(); //INICIALIZA O BARRAMENTO SPI
  rfid.PCD_Init(); //INICIALIZA MFRC522

  s.attach(pinoServo); //ASSOCIAÇÃO DO PINO DIGITAL AO OBJETO DO TIPO SERVO
  s.write(6); //INICIA O MOTOR NA POSIÇÃO 0º

  pinMode(pwm, INPUT);
  pinMode(led_verd, OUTPUT);
  pinMode(led_verm, OUTPUT);
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  analogWrite(pwm, 125);
  

  Serial.println(" Tag 1 iniciou com: " + String(creditotag1));
  Serial.println("Tag 2 iniciou com: " + String(creditotag2));
  Serial.println("--------------------------------------");

  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("BEM");
  lcd.setCursor(5, 1);
  lcd.print("VINDO");

} //fim setup


void abrecancela() {
    analogWrite(led_verm, 0);
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("ACESSO");
    lcd.setCursor(3, 1);
    lcd.print("LIBERADO");
    analogWrite(led_verd, 50);

   //bloco cancela abrindo
    for(pos = anguloCancelaFechada; pos < anguloCancelaAberta; pos++){ //PARA "pos" IGUAL A "anguloCancelaFechada",
      //ENQUANTO "pos" MENOR QUE "anguloCancelaAberta", INCREMENTA "pos"
      s.write(pos); //ESCREVE O VALOR DA POSIÇÃO QUE O SERVO DEVE GIRAR
      delay(5); //INTERVALO DE 15 MILISEGUNDOS
    }

    isCancelaOpen = true;
}

void fechacancela(){
// bloco caancela fechando
  for(pos = anguloCancelaAberta; pos >= anguloCancelaFechada; pos--){ //PARA "pos" IGUAL A "anguloCancelaFechada",
      //ENQUANTO "pos" MENOR QUE "anguloCancelaAberta", INCREMENTA "pos"
      s.write(pos); //ESCREVE O VALOR DA POSIÇÃO QUE O SERVO DEVE GIRAR
      delay(10); //INTERVALO DE 15 MILISEGUNDOS

    analogWrite(led_verd, 0);
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("PARE");
    lcd.setCursor(5, 1);
    lcd.print("");
    analogWrite(led_verm, 5);
  }

  isCancelaOpen = false;
}


float ultrasonico(){
    //Le as informacoes do sensor, em cm e pol
  float cmMsec, inMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  return cmMsec;
  inMsec = ultrasonic.convert(microsec, Ultrasonic::IN);
}

void loop() {

  float dist = ultrasonico();


  if (isCancelaOpen == true && dist > 10.0) {
    fechacancela();
  }
  
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) //VERIFICA SE O CARTÃO PRESENTE NO LEITOR É DIFERENTE DO ÚLTIMO CARTÃO LIDO. CASO NÃO SEJA, FAZ
    return; //RETORNA PARA LER NOVAMENTE

  

  /***INICIO BLOCO DE CÓDIGO RESPONSÁVEL POR GERAR A TAG RFID LIDA ***/
  String strID = "";
  for (byte i = 0; i < 4; i++) {
    strID +=
    (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(rfid.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }

  strID.toUpperCase();
  /***FIM DO BLOCO DE CÓDIGO RESPONSÁVEL POR GERAR A TAG RFID LIDA***/
 
 

  if(strID == "A7:9C:24:3C" && creditotag1 > 0){
    abrecancela();
    delay(3000);
    creditotag1 -= 10;
  }else if(strID == "93:32:EB:24" && creditotag2 > 0){
    abrecancela();
    delay(3000);
    creditotag2 -= 10;
  }else if(strID != "A7:9C:24:3C" && strID != "93:32:EB:24"){
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("VOCE NAO ESTA");
    lcd.setCursor(2, 1);
    lcd.print("CADASTRADO");
    delay(5000);
  }else if(strID == "A7:9C:24:3C" && creditotag1 <= 0){
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("VOCE NAO");
    lcd.setCursor(2, 1);
    lcd.print("POSSUI CREDITO");
    delay(5000);
  }else if(strID == "93:32:EB:24" && creditotag2 <= 0){
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("VOCE NAO");
    lcd.setCursor(2, 1);
    lcd.print("POSSUI CREDITO");
    delay(5000);
  }
  Serial.println(" Tag 1 tem: " + String(creditotag1));
  Serial.println("Tag 2 tem: " + String(creditotag2));
  Serial.print("Identificador (UID) da tag: "); //IMPRIME O TEXTO NA SERIAL
  Serial.println(strID); //IMPRIME NA SERIAL O UID DA TAG RFID
  Serial.println("---------------------------------------------------");

  rfid.PICC_HaltA(); //PARADA DA LEITURA DO CARTÃO
  rfid.PCD_StopCrypto1(); //PARADA DA CRIPTOGRAFIA NO PCD
}
