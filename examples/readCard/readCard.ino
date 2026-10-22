/*
 * Bonezegei_PN5180 - ReadCard
 * 
 * Hardware Connection Table:
 * ------------------------------------------------------------------------------------
 * PN5180 Module Pin | Arduino Uno / Nano | ESP32   | Description
 * ------------------+--------------------+---------+----------------------------------
 * VCC               | 3.3V & 5V          | 3.3V/5V | Digital: 3.3V; RF stage: 5V
 * GND               | GND                | GND     | Ground
 * NSS (CS)          | D10                | GPIO 5  | SPI Chip Select
 * MOSI              | D11                | GPIO 23 | SPI Data In
 * MISO              | D12                | GPIO 19 | SPI Data Out
 * SCK               | D13                | GPIO 18 | SPI Clock
 * BUSY              | D9                 | GPIO 4  | PN5180 Busy State Input
 * RST               | D8                 | GPIO 2  | Reset Output Pin
 * ------------------------------------------------------------------------------------
 */

#include <Bonezegei_PN5180.h>

#define PN5180_NSS  10
#define PN5180_BUSY 9
#define PN5180_RST  8

Bonezegei_PN5180 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Initializing Bonezegei PN5180...");

    if (nfc.begin()) {
        Serial.println("PN5180 initialized successfully!");
    } else {
        Serial.println("Failed to initialize PN5180.");
    }
}

void loop() {
    uint8_t uid[10];
    uint8_t uidLength = 0;

    if (nfc.readCardSerial(uid, &uidLength)) {
        Serial.print("Card Detected! UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
            if (uid[i] < 0x10) Serial.print("0");
            Serial.print(uid[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        delay(1000); // Prevent duplicate prints
    }

    delay(200);
}