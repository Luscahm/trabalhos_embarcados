
#include <SPI.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h> 

#define SS_PIN 10 //PIN SDA
#define RST_PIN 9 //PIN RESET
#define I2C_ADDR    0x27
#define LCD_COLUMNS 16
#define LCD_LINES   2

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);


MFRC522 rfid(SS_PIN, RST_PIN); //PASSAGEM DE PARÂMETROS REFERENTE AOS PINOS
const int pinoLedVerde = 3; //LED VERDE
const int pinoLedVermelho = 2; //LED VERMELHO


void setup() {
  Wire.begin(); //INICIALIZA A BIBLIOTECA WIRE
  lcd.init();
  lcd.backlight();
  Serial.begin(9600); //INICIALIZA A SERIAL
  SPI.begin(); //INICIALIZA O BARRAMENTO SPI
  rfid.PCD_Init(); //INICIALIZA MFRC522
  pinMode(pinoLedVerde, OUTPUT);
  pinMode(pinoLedVermelho, OUTPUT); 
  
  digitalWrite(pinoLedVerde, LOW); 
  digitalWrite(pinoLedVermelho, LOW);

  Serial.println("Leitor RFID iniciado. Aproxime uma tag/cartão para leitura...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) //VERIFICA SE O CARTÃO PRESENTE NO LEITOR É DIFERENTE DO ÚLTIMO CARTÃO LIDO. CASO NÃO SEJA, FAZ
    return;
    
  /*if (!rfid.PICC_IsNewCardPresent()) {
    Serial.println("Aguardando a aproximação de uma tag...");
    delay(1000); // Atraso para evitar mensagens excessivas
    return;
  }
    
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Falha na leitura da tag.");
    return;
  }
  */
  String strID = "";
  for (byte i = 0; i < 4; i++) {
    strID +=
    (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(rfid.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }
  strID.toUpperCase();

  Serial.print("Identificador (UID) da tag: "); 
  Serial.println(strID); 

  if (strID.indexOf("73:B0:2B:A6") >= 0) { // Autoriza a tag 
    digitalWrite(pinoLedVerde, HIGH); //LIGA O LED VERDE
    lcd.setCursor(1, 0);
    lcd.print("Acesso Liberado!");
    delay(3000); 
    digitalWrite(pinoLedVerde, LOW); 
  }else{ //Senão, tag não autorizada
    digitalWrite(pinoLedVermelho, HIGH); 
    lcd.setCursor(1, 0);
    lcd.print("Acesso Negado!");
    delay(3000); 
    digitalWrite(pinoLedVermelho, LOW); 
  }

  rfid.PICC_HaltA(); //PARADA DA LEITURA DO CARTÃO
  rfid.PCD_StopCrypto1(); //PARADA DA CRIPTOGRAFIA NO PCD
}
