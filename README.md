# RFID Logger

## Overview
This project is an **RFID-based logging system** using an **ESP32**, **MFRC522 RFID module**, and a web server. The system reads RFID tags, sends the UID to a server, and provides an audible indication using a buzzer. The data is logged on the server for further processing.

![WhatsApp Image 2025-03-19 at 19 32 24](https://github.com/user-attachments/assets/60d189e3-ca38-43fd-810b-814e3904a039)


---

## Project Structure
The project consists of multiple folders and scripts for different functionalities:

```
C:.
├───Main         # Main project logic
├───RFIDInput    # Handles RFID tag input processing
├───RFIDRead     # Reads RFID tags and extracts UID
└───RFIDWrite    # Writes data to RFID tags
```

---

## Hardware Requirements
- **ESP32**
- **MFRC522 RFID Reader Module**
- **RFID Tags**
- **Buzzer**
- **Jumper Wires**

---

## Software Requirements
- **Arduino IDE** (for writing and uploading code to ESP32)
- **Arduino Libraries:**
  - `WiFi.h` (for Wi-Fi connectivity)
  - `HTTPClient.h` (for sending HTTP requests)
  - `SPI.h` (for SPI communication with RFID module)
  - `MFRC522.h` (for RFID operations)
- **Web Server (Local or Cloud-Based)** to receive RFID logs

---

## Installation & Setup

### **1. Clone the Repository**
```sh
git clone https://github.com/YourUsername/RFID_Logger.git
cd RFID_Logger
```

### **2. Install Dependencies**
Make sure you have the required **Arduino libraries** installed. You can install them via the Arduino Library Manager.

### **3. Flash the Code to ESP32**
1. Open the `Main` folder in Arduino IDE.
2. Connect ESP32 to your PC.
3. Configure **Board:** `ESP32 Dev Module`.
4. Select the correct **COM Port**.
5. Click **Upload**.

---

## Code Explanation
The main functionality is implemented in the following Arduino script:

### **RFID Logger Code** (Located in `Main`)
```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22
#define BUZZER_PIN 4 // Pin connected to the buzzer

const char* ssid = "YourSSID";
const char* password = "YourWiFiPassword";

String serverUrl = "http://your-server-address/logger/test_data.php";

MFRC522 rfid(SS_PIN, RST_PIN);
String uidString;
String lastUid = "";

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  connectWiFi();
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Place an RFID tag near the reader...");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  char uidBuffer[20];
  for (byte i = 0; i < rfid.uid.size; i++) {
    snprintf(uidBuffer + i * 2, sizeof(uidBuffer) - i * 2, "%02X", rfid.uid.uidByte[i]);
  }
  uidString = String(uidBuffer);

  if (uidString == lastUid) {
    Serial.println("Duplicate UID detected. Skipping...");
    return;
  }

  lastUid = uidString;
  Serial.print("Tag UID: ");
  Serial.println(uidString);
  buzzBuzzer();
  rfid.PICC_HaltA();
  sendUidToServer(uidString);
  delay(1000);
}
```

---

## Functions Explained

- `connectWiFi()` → Connects ESP32 to the specified Wi-Fi network.
- `sendUidToServer(String uid)` → Sends UID to the server using HTTP POST.
- `buzzBuzzer()` → Activates buzzer for 200ms to indicate successful RFID scan.
- `loop()` → Continuously checks for new RFID tags and processes UID.

---

## API Endpoint
The ESP32 sends UID data to a web server. Example request:
```http
POST http://your-server-address/logger/test_data.php
Content-Type: application/x-www-form-urlencoded

UID=12345678
```

The server should handle the request and store the UID in a database.

---

## Troubleshooting
- **ESP32 not connecting to Wi-Fi?**
  - Check if the **SSID & Password** in the code are correct.
  - Ensure the Wi-Fi network is running and has internet access.
- **RFID not reading tags?**
  - Make sure the **MFRC522 module is properly wired**.
  - Check if the **UID is correctly extracted and displayed** in the serial monitor.
- **Data not reaching the server?**
  - Verify if the **server IP is reachable**.
  - Check if the **PHP endpoint is correctly handling the request**.

---

## Future Enhancements
- Add an **OLED Display** to show tag status.
- Implement **database logging** for UID tracking.
- Encrypt **Wi-Fi credentials** for security.
- Add **RFID tag expiration** for access control systems.

---

## License
This project is open-source and available under the **MIT License**.

---

## Author
Developed by **Your Name** 🚀

