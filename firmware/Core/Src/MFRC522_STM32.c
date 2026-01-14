#include "MFRC522_STM32.h"
#include "main.h"

uint8_t atqa[2];

void MFRC522_Init(MFRC522_t *dev) {
    USER_LOG("MFRC522 Min Init started");
    // Hardware reset
    HAL_GPIO_WritePin(dev->rstPort, dev->rstPin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(dev->rstPort, dev->rstPin, GPIO_PIN_SET);
    HAL_Delay(50);

    // Soft reset
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_SoftReset);
    HAL_Delay(50);

    // Clear interrupts
    MFRC522_WriteReg(dev, PCD_ComIrqReg, 0x7F);

    // Flush FIFO
    MFRC522_WriteReg(dev, PCD_FIFOLevelReg, 0x80);

    // Timer: ~25ms timeout
    MFRC522_WriteReg(dev, PCD_TModeReg, 0x80);      // Timer starts immediately
    MFRC522_WriteReg(dev, PCD_TPrescalerReg, 0xA9); // 80kHz clock
    MFRC522_WriteReg(dev, PCD_TReloadRegH, 0x03);   // 1000 ticks = ~12.5ms
    MFRC522_WriteReg(dev, PCD_TReloadRegL, 0xE8);

    // RF settings
    MFRC522_WriteReg(dev, PCD_TxAutoReg, 0x40);     // 100% ASK modulation
    MFRC522_WriteReg(dev, PCD_RFCfgReg, 0x7F);      // Max gain (48dB)
    MFRC522_WriteReg(dev, PCD_DemodReg, 0x4D);      // Sensitivity for clones

    // Enable antenna
    MFRC522_AntennaOn(dev);
    HAL_Delay(10);  // Let RF stabilize

    uint8_t version = MFRC522_ReadReg(dev, PCD_VersionReg);
    if ((version != 0x91)||(version != 0x92)){
    	USER_LOG("Version: 0x%02X (counterfeit OK for UID)", version);
    }
    else USER_LOG("Version: 0x%02X", version);
    uint8_t txCtrl = MFRC522_ReadReg(dev, PCD_TxControlReg);
    DEBUG_LOG("TxControlReg: 0x%02X (expect >= 0x03)", txCtrl);
    USER_LOG("MFRC522 Min Init complete");
	(void) txCtrl;
}

void MFRC522_AntennaOff(MFRC522_t *dev) {
    MFRC522_ClearBitMask(dev, PCD_TxControlReg, 0x03);
    DEBUG_LOG("Antenna off");
}

void MFRC522_AntennaOn(MFRC522_t *dev) {
    MFRC522_SetBitMask(dev, PCD_TxControlReg, 0x03);
    DEBUG_LOG("Antenna on");
}

uint8_t MFRC522_ReadReg(MFRC522_t *dev, uint8_t reg) {
    uint8_t addr = ((reg << 1) & 0x7E) | 0x80;
    uint8_t val = 0;
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(dev->hspi, &addr, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(dev->hspi, &val, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET);
    HAL_Delay(1);
    DEBUG_LOG("ReadReg: 0x%02X -> 0x%02X", reg, val);
    return val;
}

void MFRC522_WriteReg(MFRC522_t *dev, uint8_t reg, uint8_t value) {
    uint8_t addr = (reg << 1) & 0x7E;
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(dev->hspi, &addr, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(dev->hspi, &value, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET);
    HAL_Delay(1);
    DEBUG_LOG("WriteReg: 0x%02X = 0x%02X", reg, value);
}

void MFRC522_SetBitMask(MFRC522_t *dev, uint8_t reg, uint8_t mask) {
    uint8_t tmp = MFRC522_ReadReg(dev, reg);
    MFRC522_WriteReg(dev, reg, tmp | mask);
    DEBUG_LOG("SetBitMask: 0x%02X |= 0x%02X", reg, mask);
}

void MFRC522_ClearBitMask(MFRC522_t *dev, uint8_t reg, uint8_t mask) {
    uint8_t tmp = MFRC522_ReadReg(dev, reg);
    MFRC522_WriteReg(dev, reg, tmp & (~mask));
    DEBUG_LOG("ClearBitMask: 0x%02X &= ~0x%02X", reg, mask);
}

/*
uint8_t MFRC522_RequestA(MFRC522_t *dev, uint8_t *atqa) {
    DEBUG_LOG("RequestA");
    MFRC522_AntennaOff(dev);  // Reset RF
    HAL_Delay(5);  // Allow chip to stabilize
    MFRC522_AntennaOn(dev);
    HAL_Delay(5);  // Ensure RF is ready
    MFRC522_WriteReg(dev, PCD_ComIrqReg, 0x7F);      // Clear IRQs
    MFRC522_WriteReg(dev, PCD_FIFOLevelReg, 0x80);   // Flush FIFO
    MFRC522_WriteReg(dev, PCD_BitFramingReg, 0x07);  // 7 bits for REQA
    MFRC522_WriteReg(dev, PCD_FIFODataReg, PICC_REQA);
    HAL_Delay(2);  // Increased for counterfeit chip stability
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Transceive);
    MFRC522_SetBitMask(dev, PCD_BitFramingReg, 0x80);

    // Poll for completion (25ms timeout)
    uint32_t timeout = HAL_GetTick() + 25;
    while (HAL_GetTick() < timeout) {
        uint8_t status2 = MFRC522_ReadReg(dev, PCD_Status2Reg);
        if (status2 & 0x01) {  // Command complete
            uint8_t err = MFRC522_ReadReg(dev, PCD_ErrorReg);
            if (err & 0x1D) {  // Protocol/parity/buffer errors
                DEBUG_LOG("RequestA error: 0x%02X", err);
                MFRC522_AntennaOff(dev);
                HAL_Delay(5);
                MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle); // Stop command
                return STATUS_ERROR;
            }
            uint8_t fifoLvl = MFRC522_ReadReg(dev, PCD_FIFOLevelReg);
            if (fifoLvl >= 2) {  // ATQA is 2 bytes
                atqa[0] = MFRC522_ReadReg(dev, PCD_FIFODataReg);
                atqa[1] = MFRC522_ReadReg(dev, PCD_FIFODataReg);
                DEBUG_LOG("RequestA ATQA: 0x%02X 0x%02X", atqa[0], atqa[1]);
                MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle); // Stop command
                HAL_Delay(2);  // Post-command delay
                return STATUS_OK;
            }
            DEBUG_LOG("RequestA bad FIFO level: %d", fifoLvl);
            MFRC522_AntennaOff(dev);
            HAL_Delay(5);
            MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
            return STATUS_ERROR;
        }
        HAL_Delay(1);  // Mimic debug log timing
    }
    DEBUG_LOG("RequestA timeout");
    MFRC522_AntennaOff(dev);
    HAL_Delay(5);
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
    return STATUS_TIMEOUT;
}

uint8_t MFRC522_Anticoll(MFRC522_t *dev, uint8_t *uid) {  // Returns 4-byte UID + BCC
    DEBUG_LOG("Anticoll");
    MFRC522_WriteReg(dev, PCD_ComIrqReg, 0x7F);      // Clear IRQs
    MFRC522_WriteReg(dev, PCD_FIFOLevelReg, 0x80);   // Flush FIFO
    MFRC522_WriteReg(dev, PCD_BitFramingReg, 0x00);  // Full frame
    MFRC522_WriteReg(dev, PCD_FIFODataReg, PICC_SEL_CL1);  // 0x93
    MFRC522_WriteReg(dev, PCD_FIFODataReg, 0x20);    // Fixed CRC
    HAL_Delay(2);  // Delay for stability
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Transceive);
    MFRC522_SetBitMask(dev, PCD_BitFramingReg, 0x80);

    uint32_t timeout = HAL_GetTick() + 25;
    while (HAL_GetTick() < timeout) {
        uint8_t status2 = MFRC522_ReadReg(dev, PCD_Status2Reg);
        if (status2 & 0x01) {  // Command complete
            uint8_t err = MFRC522_ReadReg(dev, PCD_ErrorReg);
            if (err & 0x1D) {
                DEBUG_LOG("Anticoll error: 0x%02X", err);
                MFRC522_AntennaOff(dev);
                HAL_Delay(5);
                MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
                return STATUS_ERROR;
            }
            uint8_t fifoLvl = MFRC522_ReadReg(dev, PCD_FIFOLevelReg);
            if (fifoLvl == 5) {  // 4-byte UID + BCC
                for (int i = 0; i < 5; i++) {
                    uid[i] = MFRC522_ReadReg(dev, PCD_FIFODataReg);
                }
                // Validate BCC
                uint8_t calcBcc = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
                if (uid[4] != calcBcc) {
                    DEBUG_LOG("Anticoll bad BCC: calc=0x%02X, got=0x%02X", calcBcc, uid[4]);
                    MFRC522_AntennaOff(dev);
                    HAL_Delay(5);
                    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
                    return STATUS_ERROR;
                }
                DEBUG_LOG("Anticoll UID: %02X %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3], uid[4]);
                MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
                HAL_Delay(2);  // Post-command delay
                return STATUS_OK;
            }
            DEBUG_LOG("Anticoll bad FIFO level: %d", fifoLvl);
            MFRC522_AntennaOff(dev);
            HAL_Delay(5);
            MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
            return STATUS_ERROR;
        }
        HAL_Delay(1);  // Mimic debug log timing
    }
    DEBUG_LOG("Anticoll timeout");
    MFRC522_AntennaOff(dev);
    HAL_Delay(5);
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
    return STATUS_TIMEOUT;
}

uint8_t MFRC522_ReadUid(MFRC522_t *dev, uint8_t *uid) {  // Output: uid[4]
    DEBUG_LOG("Reading UID...");
    // Card detected, read UID
    uint8_t rawUid[5];
    if (MFRC522_Anticoll(dev, rawUid) != STATUS_OK) {
    	DEBUG_LOG("Anticollision failed");
        return STATUS_ERROR;
    }
    // Copy UID (drop BCC)
    for (int i = 0; i < 4; i++) {
        uid[i] = rawUid[i];
    }
    DEBUG_LOG("Card UID: %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
    return STATUS_OK;
}

uint8_t waitcardRemoval (MFRC522_t *dev){
    USER_LOG("Waiting for card removal...");
    while (1) {
        if (MFRC522_RequestA(dev, atqa) != STATUS_OK) {
        	USER_LOG("Card removed");
            return STATUS_OK; // Card removed, return success
        }
        HAL_Delay(100); // Poll every 100ms to check if card is still present
    }
}

uint8_t waitcardDetect (MFRC522_t *dev){
	atqa[0] = atqa[1] = 0;
	USER_LOG("Waiting for the card...");
	while (1){
	    if (MFRC522_RequestA(dev, atqa) == STATUS_OK) {
	    	USER_LOG("Card detected");
	        return STATUS_OK;
	    }
	    HAL_Delay(100);	// Poll every 100ms to check if card is  present
	}
}
*/

uint8_t MFRC522_ToCard(MFRC522_t *dev, uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint32_t *backLen)
{
  uint8_t status = MI_ERR;
  uint8_t irqEn = 0x00;
  uint8_t waitIRq = 0x00;
  uint8_t lastBits;
  uint8_t n;
  uint32_t i;

	switch (command)
	{
	case PCD_AUTHENT:     // Certification cards close
	{
		irqEn = 0x12;
		waitIRq = 0x10;
		break;
	}
	case PCD_TRANSCEIVE:  // Transmit FIFO data
	{
		irqEn = 0x77;
		waitIRq = 0x30;
		break;
	}
	default:
		break;
	}

	MFRC522_WriteReg(dev, CommIEnReg, irqEn|0x80);  // Interrupt request
	MFRC522_ClearBitMask(dev, CommIrqReg, 0x80);         // Clear all interrupt request bit
	MFRC522_SetBitMask(dev, FIFOLevelReg, 0x80);         // FlushBuffer=1, FIFO Initialization
	
	MFRC522_WriteReg(dev, CommandReg, PCD_IDLE);
	
	for (i=0; i<sendLen; i++)
	{
		MFRC522_WriteReg(dev, FIFODataReg, sendData[i]);
	}

    // HAL_Delay(2);  // Increased for counterfeit chip stability

	MFRC522_WriteReg(dev, CommandReg, command);
	if (command == PCD_TRANSCEIVE)
	{
		MFRC522_SetBitMask(dev, BitFramingReg, 0x80);      // StartSend=1,transmission of data starts
	}


    // Poll for completion (25ms timeout)
	i = HAL_GetTick() + 25;
	do
	{
		n = MFRC522_ReadReg(dev, CommIrqReg);	
	}
	while ((HAL_GetTick() < i) && !(n&0x01) && !(n&waitIRq));

	if (HAL_GetTick() < i)
	{
		if(!(MFRC522_ReadReg(dev, ErrorReg) & 0x1B))  // BufferOvfl Collerr CRCErr ProtecolErr
		{
			status = MI_OK;
			if (n & irqEn & 0x01)
			{
				status = MI_NOTAGERR;             // ??
			}
			if (command == PCD_TRANSCEIVE)
			{
				n = MFRC522_ReadReg(dev, FIFOLevelReg);
				lastBits = MFRC522_ReadReg(dev, ControlReg) & 0x07;
				if (lastBits)
				{
					*backLen = (n-1)*8 + lastBits;
				}
				else
				{
					*backLen = n*8;
				}
				if (n == 0)
				{
					n = 1;
				}
				if (n > MAX_LEN)
				{
					n = MAX_LEN;
				}
				// Reading the received data in FIFO
				for (i=0; i<n; i++)
				{
					backData[i] = MFRC522_ReadReg(dev, FIFODataReg);
				}
			}
		}
		else
		{
			//printf("~~~ buffer overflow, collerr, crcerr, or protecolerr\r\n");
			status = MI_ERR;
		}
	}
	else
	{
		//printf("~~~ request timed out\r\n");
	}

	return status;
}

uint8_t MFRC522_Request(MFRC522_t *dev, uint8_t req_mode, uint8_t *tag_type)
{
	uint8_t status;
	uint32_t backBits; // The received data bits

	MFRC522_WriteReg(dev, BitFramingReg, 0x07);   // TxLastBists = BitFramingReg[2..0]

	tag_type[0] = req_mode;

	status = MFRC522_ToCard(dev, PCD_TRANSCEIVE, tag_type, 1, tag_type, &backBits);
	if ((status != MI_OK) || (backBits != 0x10)) {
		status = MI_ERR;
	}

	return status;
}

uint8_t MFRC522_Anticoll(MFRC522_t *dev, uint8_t *ser_num)
{
	uint8_t status;
	uint8_t i;
	uint8_t ser_num_check=0;
	uint32_t un_len;

	//ClearBitMask(Status2Reg, 0x08);		//TempSensclear
	//ClearBitMask(CollReg,0x80);			//ValuesAfterColl
	MFRC522_WriteReg(dev, BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]

	ser_num[0] = PICC_ANTICOLL;
	ser_num[1] = 0x20;
	status = MFRC522_ToCard(dev, PCD_TRANSCEIVE, ser_num, 2, ser_num, &un_len);

	if (status == MI_OK)
	{
	//Check card serial number
		for (i=0; i<4; i++)
		{
			ser_num_check ^= ser_num[i];
		}
		if (ser_num_check != ser_num[i])
		{
			status = MI_ERR;
		}
	}
	//SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1

	return status;
}

void MFRC522_CalculateCRC(MFRC522_t *dev, uint8_t *pIndata, uint8_t len, uint8_t *pOutData)
{
  uint8_t i, n;

  MFRC522_ClearBitMask(dev, DivIrqReg, 0x04);			//CRCIrq = 0
  MFRC522_SetBitMask(dev, FIFOLevelReg, 0x80);			//Clear the FIFO pointer
  //Write_MFRC522(CommandReg, PCD_IDLE);

  //Writing data to the FIFO
  for (i=0; i<len; i++)
  {
    MFRC522_WriteReg(dev, FIFODataReg, *(pIndata+i));
  }
  MFRC522_WriteReg(dev, CommandReg, PCD_CALCCRC);

  //Wait CRC calculation is complete
  i = 0xFF;
  do
  {
    n = MFRC522_ReadReg(dev, DivIrqReg);
    i--;
  }
  while ((i!=0) && !(n&0x04));			//CRCIrq = 1

  //Read CRC calculation result
  pOutData[0] = MFRC522_ReadReg(dev, CRCResultRegL);
  pOutData[1] = MFRC522_ReadReg(dev, CRCResultRegM);
}

uint8_t MFRC522_SelectTag(MFRC522_t *dev, uint8_t *serNum)
{
	uint8_t i;
	uint8_t status;
	uint8_t size;
	uint32_t recvBits;
	uint8_t buffer[9];

	//ClearBitMask(Status2Reg, 0x08);			//MFCrypto1On=0

	buffer[0] = PICC_SELECTTAG;
	buffer[1] = 0x70;
	for (i=0; i<5; i++)
	{
		buffer[i+2] = *(serNum+i);
	}
	MFRC522_CalculateCRC(dev, buffer, 7, &buffer[7]);		//??
	status = MFRC522_ToCard(dev, PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18))
	{
		size = buffer[0];
	}
	else
	{
		size = 0;
	}

	return size;
}

uint8_t MFRC522_Auth(MFRC522_t *dev, uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum)
{
	uint8_t status;
	uint32_t recvBits;
	uint8_t i;
	uint8_t buff[12];

	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i=0; i<6; i++)
	{
		buff[i+2] = *(Sectorkey+i);
	}
	for (i=0; i<4; i++)
	{
		buff[i+8] = *(serNum+i);
	}
	status = MFRC522_ToCard(dev, PCD_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK) || (!(MFRC522_ReadReg(dev, Status2Reg) & 0x08)))
	{
		status = MI_ERR;
	}

	return status;
}

uint8_t MFRC522_Read(MFRC522_t *dev, uint8_t block_addr, uint8_t *recv_data)
{
	uint8_t status;
	uint32_t un_len;

	recv_data[0] = PICC_READ;
	recv_data[1] = block_addr;
	MFRC522_CalculateCRC(dev, recv_data,2, &recv_data[2]);
	status = MFRC522_ToCard(dev, PCD_TRANSCEIVE, recv_data, 4, recv_data, &un_len);

	if ((status != MI_OK) || (un_len != 0x90))
	{
		status = MI_ERR;
	}

	return status;
}

void print_buf(uint8_t *buf, uint32_t len)
{
	printf("Buf: ");
	uint32_t i = 0;
	while (i < len)
	{
		printf(" %02X", buf[i]);
		i++;
	}
	printf("\n");
}
uint8_t MFRC522_Write(MFRC522_t *dev, uint8_t blockAddr, uint8_t *writeData)
{
	uint8_t status;
	uint32_t recvBits;
	uint8_t i;
	uint8_t buff[18];

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	MFRC522_CalculateCRC(dev, buff, 2, &buff[2]);
	status = MFRC522_ToCard(dev, PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK))// || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
	{
		status = MI_ERR;
	}
	if (status == MI_OK)
	{
		for (i=0; i<16; i++)		//Data to the FIFO write 16Byte
		{
			buff[i] = *(writeData+i);
		}
		MFRC522_CalculateCRC(dev, buff, 16, &buff[16]);
		status = MFRC522_ToCard(dev, PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
		// print_buf(buff, 18);
		if ((status != MI_OK))// || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
		{
			status = MI_ERR;
		}
	}

	return status;
}

void MFRC522_Halt(MFRC522_t *dev)
{
	uint8_t status;
	uint32_t unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	MFRC522_CalculateCRC(dev, buff, 2, &buff[2]);

	status = MFRC522_ToCard(dev, PCD_TRANSCEIVE, buff, 4, buff,&unLen);
	(void) status;
	//return status;
}
//--------------------------------------
void MFRC522_StopCrypto1(MFRC522_t *dev)
{
	MFRC522_ClearBitMask(dev, Status2Reg, 0x08);
}
