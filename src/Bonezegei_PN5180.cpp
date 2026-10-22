#include "Bonezegei_PN5180.h"

Bonezegei_PN5180::Bonezegei_PN5180(uint8_t ssPin, uint8_t busyPin, uint8_t rstPin) {
    _ssPin = ssPin;
    _busyPin = busyPin;
    _rstPin = rstPin;
}

bool Bonezegei_PN5180::begin() {
    pinMode(_ssPin, OUTPUT);
    pinMode(_busyPin, INPUT);
    pinMode(_rstPin, OUTPUT);

    deselectPin();
    SPI.begin();

    reset();

    // Load default ISO14443A RF Configuration (0x00 Transmit, 0x80 Receive)
    loadRFConfig(0x00, 0x80);
    setRF(true);

    return true;
}

void Bonezegei_PN5180::reset() {
    digitalWrite(_rstPin, LOW);
    delay(10);
    digitalWrite(_rstPin, HIGH);
    delay(10);
    waitForBusy();
}

void Bonezegei_PN5180::waitForBusy(uint32_t timeoutMs) {
    uint32_t start = millis();
    while (digitalRead(_busyPin) == HIGH) {
        if (millis() - start > timeoutMs) {
            break;
        }
    }
}

void Bonezegei_PN5180::selectPin() {
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_ssPin, LOW);
}

void Bonezegei_PN5180::deselectPin() {
    digitalWrite(_ssPin, HIGH);
    SPI.endTransaction();
}

uint32_t Bonezegei_PN5180::readRegister(uint8_t reg) {
    waitForBusy();
    selectPin();
    SPI.transfer(PN5180_CMD_READ_REGISTER);
    SPI.transfer(reg);
    uint32_t val = 0;
    val |= (uint32_t)SPI.transfer(0x00);
    val |= ((uint32_t)SPI.transfer(0x00)) << 8;
    val |= ((uint32_t)SPI.transfer(0x00)) << 16;
    val |= ((uint32_t)SPI.transfer(0x00)) << 24;
    deselectPin();
    waitForBusy();
    return val;
}

void Bonezegei_PN5180::writeRegister(uint8_t reg, uint32_t val) {
    waitForBusy();
    selectPin();
    SPI.transfer(PN5180_CMD_WRITE_REGISTER);
    SPI.transfer(reg);
    SPI.transfer(val & 0xFF);
    SPI.transfer((val >> 8) & 0xFF);
    SPI.transfer((val >> 16) & 0xFF);
    SPI.transfer((val >> 24) & 0xFF);
    deselectPin();
    waitForBusy();
}

void Bonezegei_PN5180::writeRegisterOrMask(uint8_t reg, uint32_t mask) {
    waitForBusy();
    selectPin();
    SPI.transfer(PN5180_CMD_WRITE_REGISTER_OR_MASK);
    SPI.transfer(reg);
    SPI.transfer(mask & 0xFF);
    SPI.transfer((mask >> 8) & 0xFF);
    SPI.transfer((mask >> 16) & 0xFF);
    SPI.transfer((mask >> 24) & 0xFF);
    deselectPin();
    waitForBusy();
}

void Bonezegei_PN5180::writeRegisterAndMask(uint8_t reg, uint32_t mask) {
    waitForBusy();
    selectPin();
    SPI.transfer(PN5180_CMD_WRITE_REGISTER_AND_MASK);
    SPI.transfer(reg);
    SPI.transfer(mask & 0xFF);
    SPI.transfer((mask >> 8) & 0xFF);
    SPI.transfer((mask >> 16) & 0xFF);
    SPI.transfer((mask >> 24) & 0xFF);
    deselectPin();
    waitForBusy();
}

void Bonezegei_PN5180::loadRFConfig(uint8_t txConfig, uint8_t rxConfig) {
    waitForBusy();
    selectPin();
    SPI.transfer(PN5180_CMD_LOAD_RF_CONFIG);
    SPI.transfer(txConfig);
    SPI.transfer(rxConfig);
    deselectPin();
    waitForBusy();
}

void Bonezegei_PN5180::setRF(bool state) {
    if (state) {
        writeRegisterOrMask(PN5180_REG_SYSTEM_CONFIG, PN5180_SYS_CONFIG_RF_ON);
    } else {
        writeRegisterAndMask(PN5180_REG_SYSTEM_CONFIG, ~PN5180_SYS_CONFIG_RF_ON);
    }
}

bool Bonezegei_PN5180::transceiveCommand(const uint8_t *sendData, uint8_t sendLen, uint8_t *recvData, uint8_t *recvLen, uint8_t validBits) {
    writeRegister(PN5180_REG_IRQ_CLEAR, 0xFFFFFFFF);

    waitForBusy();
    selectPin();
    SPI.transfer(PN5180_CMD_SEND_DATA);
    SPI.transfer(validBits);
    for (uint8_t i = 0; i < sendLen; i++) {
        SPI.transfer(sendData[i]);
    }
    deselectPin();
    waitForBusy();

    waitForBusy();
    selectPin();
    SPI.transfer(PN5180_CMD_READ_DATA);
    SPI.transfer(0x00);
    uint8_t bytesRead = 0;
    while (bytesRead < *recvLen) {
        recvData[bytesRead] = SPI.transfer(0x00);
        bytesRead++;
    }
    deselectPin();
    waitForBusy();

    *recvLen = bytesRead;
    return (bytesRead > 0);
}

bool Bonezegei_PN5180::isCardPresent() {
    uint8_t reqa = 0x26;
    uint8_t response[2];
    uint8_t len = 2;

    return transceiveCommand(&reqa, 1, response, &len, 7);
}

bool Bonezegei_PN5180::readCardSerial(uint8_t *uidBuffer, uint8_t *uidSize) {
    uint8_t reqa = 0x26;
    uint8_t atqa[2];
    uint8_t len = 2;

    // 1. Send REQA (7-bit)
    if (!transceiveCommand(&reqa, 1, atqa, &len, 7)) {
        return false;
    }

    // 2. Perform ISO14443A Anticollision Loop
    uint8_t anticollisionCmd[2] = {0x93, 0x20};
    uint8_t response[5];
    len = 5;

    if (!transceiveCommand(anticollisionCmd, 2, response, &len, 0)) {
        return false;
    }

    // Verify BCC Checksum
    uint8_t bcc = response[0] ^ response[1] ^ response[2] ^ response[3];
    if (bcc != response[4]) {
        return false;
    }

    for (uint8_t i = 0; i < 4; i++) {
        uidBuffer[i] = response[i];
    }
    *uidSize = 4;

    return true;
}
