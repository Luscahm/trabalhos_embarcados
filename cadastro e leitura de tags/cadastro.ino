#include <SPI.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h> 
#include <WiFi.h>
#include <Keypad.h> // Library for keypad

#define SS_PIN  21
#define RST_PIN 22
#define I2C_ADDR    0x27
#define LCD_COLUMNS 16
#define LCD_LINES   2
const byte ROWS = 4; /* four rows */
const byte COLS = 4; /* four columns */
/* define the symbols on the buttons of the keypads */
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {14, 12, 13, 27}; /* connect to the row pinouts of the keypad */
byte colPins[COLS] = {26, 25, 33, 32}; /* connect to the column pinouts of the keypad */

/* initialize an instance of class NewKeypad */
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);


WiFiServer server(80);


const char* ssid = "";
const char* password = "";





MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

const int pinoLedVerde = 2; //LED VERDE
const int pinoLedVermelho = 4; //LED VERMELHO


void setup() {
  Wire.begin(); //INICIALIZA A BIBLIOTECA WIRE
  lcd.init();
  lcd.backlight();
  Serial.begin(115200); //INICIALIZA A SERIAL
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  String cadastrado = "03:BF:C6:A6";
  pinMode(pinoLedVerde, OUTPUT);
  pinMode(pinoLedVermelho, OUTPUT); 
  
  digitalWrite(pinoLedVerde, LOW); 
  digitalWrite(pinoLedVermelho, LOW);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectada.");
  Serial.println("Endereço de IP: ");
  Serial.println(WiFi.localIP());
  server.begin();

}

void loop() {
   /* 
   
  if (!rfid.PICC_IsNewCardPresent()) {
    Serial.println("Aguardando a aproximação de uma tag...");
    delay(1000); // Atraso para evitar mensagens excessivas
    return;
  }
  

  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Falha na leitura da tag.");
    return;
  }
  */
  char customKey = customKeypad.getKey();
  String strID = "";
  while (customKey == 'A'){
    Serial.println("Leitor RFID iniciado para cadastro.");    
      if (rfid.PICC_IsNewCardPresent() || rfid.PICC_ReadCardSerial()){ //VERIFICA SE O CARTÃO PRESENTE NO LEITOR É DIFERENTE DO ÚLTIMO CARTÃO LIDO. CASO NÃO SEJA, FAZ
          String cadastrado = "";
            for (byte i = 0; i < 4; i++) {
              strID +=
              (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
              String(rfid.uid.uidByte[i], HEX) +
              (i!=3 ? ":" : "");
            }
          strID.toUpperCase();
          
          Serial.print("Identificador (UID) da tag: "); 
          Serial.println(strID);
          rfid.PICC_HaltA(); //PARADA DA LEITURA DO CARTÃO
          rfid.PCD_StopCrypto1(); //PARADA DA CRIPTOGRAFIA NO PCD
          lcd.setCursor(1, 0);
          lcd.print("Cadastrado");
          lcd.noBacklight();
          cadastrado = strID;
          break;
      }
  }
  while(customKey == '3'){
    Serial.println("Leitor RFID iniciado para leitura. Aproxime uma tag/cartão para leitura...");

    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) //VERIFICA SE O CARTÃO PRESENTE NO LEITOR É DIFERENTE DO ÚLTIMO CARTÃO LIDO. CASO NÃO SEJA, FAZ
      return;
    String strID = "";
    String cadastrado = "";
    for (byte i = 0; i < 4; i++) {
      strID +=
      (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
      String(rfid.uid.uidByte[i], HEX) +
      (i!=3 ? ":" : "");
    }
    strID.toUpperCase();
    

    Serial.print("Identificador (UID) da tag: "); 
    Serial.println(strID);
    rfid.PICC_HaltA(); //PARADA DA LEITURA DO CARTÃO
    rfid.PCD_StopCrypto1(); //PARADA DA CRIPTOGRAFIA NO PCD
    lcd.setCursor(1, 0);
    lcd.print("AAAAAAAAAAAAAAAAAAAAAAAAA");
    lcd.noBacklight();
    WiFiClient client = server.available();
    // Serial.println(client);
    if (client){
      Serial.println("New Client.");
      String currentLine = "";
      while (client.connected()) {
        
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          if (c == '\n') {
            if (currentLine.length() == 0) {
              Serial.println("send message html");
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
              client.println("<h1>Sistemas Computacionais embarcado - ESP32 wifi</h1>");

              if(strID.indexOf("03:BF:C6:A6") >= 0){
                digitalWrite(pinoLedVerde, HIGH); //LIGA O LED VERDE
                lcd.setCursor(1, 0);
                lcd.print("Acesso Liberado!");
                delay(3000); 
                digitalWrite(pinoLedVerde, LOW); 
                client.println("<h3>Leber liberado para acesso</h3>");
            

              }
              else{ //Senão, ag não autorizada
                  digitalWrite(pinoLedVermelho, HIGH); 
                  lcd.setCursor(1, 0);
                  lcd.print("Acesso Negado!");
                  delay(3000); 
                  digitalWrite(pinoLedVermelho, LOW); 
                  client.println("<h3>Tag nao liberada pra acesso</h3>");
              }
              break;
            } else {
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;
          }
        }
      }
      client.stop();
      Serial.println("Client Disconnected.");
      break;
    }
  }

}
