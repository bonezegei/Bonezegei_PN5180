#ifndef _BONEZEGEI_PN5180_H_
#define _BONEZEGEI_PN5180_H_

#include <Arduino.h>
#include <SPI.h>

// PN5180 Direct Commands
#define PN5180_CMD_WRITE_REGISTER          0x00
#define PN5180_CMD_WRITE_REGISTER_OR_MASK  0x01
#define PN5180_CMD_WRITE_REGISTER_AND_MASK 0x02
#define PN5180_CMD_READ_REGISTER           0x04
#define PN5180_CMD_WRITE_EEPROM            0x05
#define PN5180_CMD_READ_EEPROM             0x06
#define PN5180_CMD_SEND_DATA               0x09
#define PN5180_CMD_READ_DATA               0x0A
#define PN5180_CMD_SWITCH_MODE             0x0B
#define PN5180_CMD_LOAD_RF_CONFIG          0x11

// PN5180 Register Addresses
#define PN5180_REG_SYSTEM_CONFIG           0x00
#define PN5180_REG_IRQ_STATUS              0x06
#define PN5180_REG_IRQ_CLEAR               0x08
#define PN5180_REG_IRQ_ENABLE              0x09
#define PN5180_REG_CRC_TX_CONFIG           0x18
#define PN5180_REG_CRC_RX_CONFIG           0x19

// Bit Flags
#define PN5180_SYS_CONFIG_RF_ON            (1 << 0)

class Bonezegei_PN5180 {
public:
    Bonezegei_PN5180(uint8_t ssPin, uint8_t busyPin, uint8_t rstPin);

    bool begin();
    void reset();

    // Register Read/Write
    uint32_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint32_t val);
    void writeRegisterOrMask(uint8_t reg, uint32_t mask);
    void writeRegisterAndMask(uint8_t reg, uint32_t mask);

    // RF & Configuration
    void loadRFConfig(uint8_t txConfig, uint8_t rxConfig);
    void setRF(bool state);

    // Transceive & Card Operations
    bool transceiveCommand(const uint8_t *sendData, uint8_t sendLen, uint8_t *recvData, uint8_t *recvLen, uint8_t validBits = 0);
    bool isCardPresent();
    bool readCardSerial(uint8_t *uidBuffer, uint8_t *uidSize);

private:
    uint8_t _ssPin;
    uint8_t _busyPin;
    uint8_t _rstPin;

    void waitForBusy(uint32_t timeoutMs = 100);
    void selectPin();
    void deselectPin();
};

#endif
