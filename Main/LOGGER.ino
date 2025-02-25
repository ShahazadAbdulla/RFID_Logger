#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define RST_PIN 22
#define SS_PIN 21
#define BUZZER 15

#include <TFT_eSPI.h>  // Include TFT library

TFT_eSPI tft = TFT_eSPI();  // Create TFT object

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

int blockNum = 2;

byte bufferLen = 18;
byte readBlockData[18];

String card_holder_name;
const String sheet_url = "https://script.google.com/macros/s/AKfycbzKO00jSk73AibcF6g_p0-qNkCDNDsjso_iad3tgWiYJtY7xeLOSDm1Z16nK774yHRlQQ/exec?name=";
#define WIFI_SSID "SAINTGITS"

#define WIFI_PASSWORD "saintgitswifi"

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.print("Connecting to AP");


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }


  pinMode(BUZZER, OUTPUT);
  SPI.begin();
  tft.begin();

  tft.fillRect(0, 0, 128, 160, 0x0);
  tft.setRotation(4);
}
bool c = 0;
void loop() {
  mfrc522.PCD_Init();
  placecard();
  if (!mfrc522.PICC_IsNewCardPresent()) {
     c = 1;
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
     c = 1;
    return;
  }
  c = 0;
  Serial.println();
  verifying();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  //Print the data read from block
  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  for (int j = 0; j < 16; j++) {
    Serial.write(readBlockData[j]);
  }
  Serial.println();
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    card_holder_name = sheet_url + String((char*)readBlockData);
    card_holder_name.trim();
    Serial.println(card_holder_name);
    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));

    if (https.begin(client, (String)card_holder_name)) {
      Serial.print(F("[HTTPS] GET...\n"));
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been sent and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
      verified();
      delay(1000);
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}


void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Authentication success");
  }
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Block was read successfully");
  }
}

void placecard() {
  if   (c == 0) {
      tft.fillRect(0, 0, 128, 160, 0x0);
    }

  tft.setTextColor(0xA800);
  tft.setTextSize(1);
  tft.setFreeFont();
  tft.drawString("ATTENDANCE LOGGER", 13, 8);

  tft.drawRect(0, 0, 128, 25, 0xAAA0);

  tft.drawRect(0, 55, 128, 55, 0x57FF);

  tft.setTextColor(0x57EA);
  tft.drawString("PLACE THE TAG AT", 14, 65);

  tft.setTextColor(0xAAA0);
  tft.drawString("IEDC@SAINTGITS", 22, 150);

  tft.setTextColor(0x57EA);
  tft.drawString("THE READER", 31, 83);
}

void verifying() {
  static const unsigned char PROGMEM image_download_bits[] = { 0x00, 0x40, 0x00, 0x00, 0xe0, 0x00, 0x01, 0x40, 0x00, 0x02, 0xa0, 0x00, 0x05, 0x10, 0x00, 0x0a, 0x08, 0x00, 0x14, 0x08, 0x00, 0x28, 0x08, 0x00, 0x50, 0x08, 0x00, 0xe0, 0x08, 0x00, 0x50, 0x08, 0x00, 0x08, 0x07, 0xe0, 0x07, 0xe0, 0x10, 0x00, 0x14, 0x0a, 0x00, 0x17, 0xe7, 0x00, 0x17, 0xca, 0x00, 0x17, 0x94, 0x00, 0x17, 0x28, 0x00, 0x12, 0x50, 0x00, 0x08, 0xa0, 0x00, 0x05, 0x40, 0x00, 0x02, 0x80, 0x00, 0x07, 0x00, 0x00, 0x02, 0x00 };
  tft.fillRect(0, 0, 128, 160, 0x0);

  tft.setTextColor(0xA800);
  tft.setTextSize(1);
  tft.setFreeFont();
  tft.drawString("IEDC LOGGER", 32, 10);

  tft.drawRect(0, 0, 128, 25, 0xAAA0);

  tft.drawEllipse(64, 86, 38, 35, 0xAAA0);

  tft.drawBitmap(52, 73, image_download_bits, 24, 24, 0xFFFF);

  tft.setTextColor(0x57FF);
  tft.drawString("Verifying ..", 44, 131);
}

void verified() {
  static const unsigned char PROGMEM image_download_bits[] = { 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x03, 0xf0, 0x00, 0x03, 0xf0, 0x00, 0x03, 0xc0, 0x00, 0x0f, 0xc0, 0x00, 0x0f, 0x00, 0xf0, 0x3f, 0x00, 0xf0, 0x3f, 0x00, 0xfc, 0xfc, 0x00, 0xfc, 0xfc, 0x00, 0x3f, 0xf0, 0x00, 0x3f, 0xf0, 0x00, 0x0f, 0xc0, 0x00, 0x0f, 0xc0, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00 };
  tft.fillRect(0, 0, 128, 160, 0x0);
  tft.setTextColor(0xA800);
  tft.setTextSize(1);
  tft.setFreeFont();
  tft.drawString("IEDC", 48, 12);

  tft.drawRect(0, 0, 128, 25, 0xAAA0);

  tft.drawEllipse(64, 86, 38, 35, 0x57EA);

  tft.setTextColor(0x57EA);
  tft.drawString("Verified", 42, 139);

  tft.drawBitmap(54, 77, image_download_bits, 20, 17, 0x57EA);
}