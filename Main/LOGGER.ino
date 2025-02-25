#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22
#define BUZZER_PIN 4 // Pin connected to the buzzer

const char* ssid = "S23";
const char* password = "password";

String serverUrl = "http://192.168.50.53/logger/test_data.php";

MFRC522 rfid(SS_PIN, RST_PIN); // Create an instance of the MFRC522 class
String uidString;              // Variable to store UID as a string
String lastUid = "";           // Variable to store the last UID processed

void setup() {
  Serial.begin(115200);        // Initialize serial communication
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
  connectWiFi();
  SPI.begin();                 // Initialize SPI bus
  rfid.PCD_Init();             // Initialize MFRC522 module
  Serial.println("Place an RFID tag near the reader...");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Check if a new card is present
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Read the card's UID
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Convert UID to a single string
  char uidBuffer[20];
  for (byte i = 0; i < rfid.uid.size; i++) {
    snprintf(uidBuffer + i * 2, sizeof(uidBuffer) - i * 2, "%02X", rfid.uid.uidByte[i]);
  }
  uidString = String(uidBuffer);

  // Check if this UID has already been processed
  if (uidString == lastUid) {
    Serial.println("Duplicate UID detected. Skipping...");
    return; // Skip sending the UID to the server
  }

  // Update lastUid with the current UID
  lastUid = uidString;

  Serial.print("Tag UID: ");
  Serial.println(uidString);

  // Buzz to indicate successful RFID read
  buzzBuzzer();

  // Halt communication with the card
  rfid.PICC_HaltA();

  // Send UID to server
  sendUidToServer(uidString);

  delay(1000); // Delay to avoid multiple reads
}

void sendUidToServer(String uid) {
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "UID=" + uid;
  int httpCode = http.POST(postData);

  if (httpCode > 0) {
    Serial.println("HTTP Response Code: " + String(httpCode));
    String response = http.getString();
    Serial.println("Server Response: " + response);
  } else {
    Serial.println("Error in sending POST request");
  }

  http.end(); // Close HTTP connection
}

void buzzBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH); // Turn the buzzer on
  delay(200);                     // Beep duration (200ms)
  digitalWrite(BUZZER_PIN, LOW);  // Turn the buzzer off
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to Wi-Fi...");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi. Retrying...");
    delay(5000);
  }
}
