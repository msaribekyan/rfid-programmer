#ifndef MFRC522_STM32_MIN_H
#define MFRC522_STM32_MIN_H

#include "stm32f0xx_hal.h"
#include <stdint.h>
#include <stdio.h>

#define ENABLE_USER_LOG   1
#define ENABLE_DEBUG_LOG  0 // Test with this disabled

#if ENABLE_USER_LOG
  #define USER_LOG(fmt, ...) printf("[USER] " fmt "\r\n", ##__VA_ARGS__)
#else
  #define USER_LOG(fmt, ...)
#endif

#if ENABLE_DEBUG_LOG
  #define DEBUG_LOG(fmt, ...) printf("[DEBUG] " fmt "\r\n", ##__VA_ARGS__)
#else
  #define DEBUG_LOG(fmt, ...)
#endif

// Essential registers
#define PCD_CommandReg     0x01
#define PCD_ComIrqReg      0x04
#define PCD_ErrorReg       0x06
#define PCD_Status2Reg     0x08
#define PCD_FIFODataReg    0x09
#define PCD_FIFOLevelReg   0x0A
#define PCD_BitFramingReg  0x0D
#define PCD_TxControlReg   0x14
#define PCD_TxAutoReg      0x15
#define PCD_RFCfgReg       0x26
#define PCD_TModeReg       0x2A
#define PCD_TPrescalerReg  0x2B
#define PCD_TReloadRegL    0x2C
#define PCD_TReloadRegH    0x2D
#define PCD_DemodReg       0x19
#define PCD_VersionReg     0x37

// Commands
#define PCD_Idle           0x00
#define PCD_Transceive     0x0C
#define PCD_SoftReset      0x0F

#define PCD_IDLE           0x00
#define PCD_TRANSCEIVE     0x0C
#define PCD_AUTHENT        0x0E
// PICC commands
#define PICC_REQA          0x26
#define PICC_SEL_CL1       0x93

#define PICC_REQIDL          0x26               // find the antenna area does not enter hibernation
#define PICC_REQALL          0x52               // find all the cards antenna area
#define PICC_ANTICOLL        0x93               // anti-collision
#define PICC_SElECTTAG       0x93               // election card
#define PICC_AUTHENT1A       0x60               // authentication key A
#define PICC_AUTHENT1B       0x61               // authentication key B
#define PICC_READ            0x30               // Read Block
#define PICC_WRITE           0xA0               // write block
#define PICC_DECREMENT       0xC0               // debit
#define PICC_INCREMENT       0xC1               // recharge
#define PICC_RESTORE         0xC2               // transfer block data to the buffer
#define PICC_TRANSFER        0xB0               // save the data in the buffer
#define PICC_HALT            0x50               // Sleep

// Status
#define STATUS_OK          0
#define STATUS_ERROR       1
#define STATUS_TIMEOUT     2

//Page 0:Command and Status
#define     Reserved00            0x00
#define     CommandReg            0x01
#define     CommIEnReg            0x02
#define     DivlEnReg             0x03
#define     CommIrqReg            0x04
#define     DivIrqReg             0x05
#define     ErrorReg              0x06
#define     Status1Reg            0x07
#define     Status2Reg            0x08
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F
//Page 2:CFG
#define     Reserved20            0x20
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg              0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39
#define     TestDAC2Reg           0x3A
#define     TestADCReg            0x3B
#define     Reserved31            0x3C
#define     Reserved32            0x3D
#define     Reserved33            0x3E
#define     Reserved34            0x3F

//And MF522 The error code is returned when communication
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2

#define MAX_LEN 16

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *csPort;
    uint16_t csPin;
    GPIO_TypeDef *rstPort;
    uint16_t rstPin;
} MFRC522_t;

// Prototypes
void MFRC522_Init(MFRC522_t *dev);
void MFRC522_AntennaOff(MFRC522_t *dev);
void MFRC522_AntennaOn(MFRC522_t *dev);
uint8_t MFRC522_ReadReg(MFRC522_t *dev, uint8_t reg);
void MFRC522_WriteReg(MFRC522_t *dev, uint8_t reg, uint8_t value);
void MFRC522_SetBitMask(MFRC522_t *dev, uint8_t reg, uint8_t mask);
void MFRC522_ClearBitMask(MFRC522_t *dev, uint8_t reg, uint8_t mask);
// uint8_t MFRC522_RequestA(MFRC522_t *dev, uint8_t *atqa);
// uint8_t MFRC522_Anticoll(MFRC522_t *dev, uint8_t *uid);
uint8_t MFRC522_ReadUid(MFRC522_t *dev, uint8_t *uid);
uint8_t waitcardRemoval (MFRC522_t *dev);
uint8_t waitcardDetect (MFRC522_t *dev);


uint8_t MFRC522_ToCard(MFRC522_t *dev, uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint32_t *backLen);
uint8_t MFRC522_Request(MFRC522_t *dev, uint8_t req_mode, uint8_t *tag_type);
uint8_t MFRC522_Anticoll(MFRC522_t *dev, uint8_t *ser_num);

#endif
