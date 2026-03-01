#include "rfid.h"
#include "usb.h"
#include "pn532_stm32f0.h"

uint8_t data[MAX_USB_FRAME_LENGTH];
uint8_t data_len = 0;

uint8_t rfid_init(PN532 *pn532)
{
	int ret;
	uint8_t buff[255];

	PN532_SPI_Init(pn532);
	ret = PN532_GetFirmwareVersion(pn532, buff);
	PN532_SamConfiguration(pn532);
	if (ret != PN532_STATUS_OK)
		return 1;
	return 0;
}

uint8_t rfid_mf_get_uid(PN532 *pn532, uint8_t *uid, int32_t *uid_len)
{
	*uid_len = PN532_ReadPassiveTarget(pn532, uid, PN532_MIFARE_ISO14443A, 50);
	if (*uid_len == PN532_STATUS_ERROR)
		return 1;
	return 0;	
}

uint8_t rfid_mf_rblk(PN532 *pn532, uint8_t *uid, int32_t uid_len, uint8_t blk, uint8_t *key, uint8_t *data)
{
	uint32_t pn532_error;

   	pn532_error = PN532_MifareClassicAuthenticateBlock(pn532, uid, uid_len,
			6, MIFARE_CMD_AUTH_A, key);
	if (pn532_error)
	{
		pn532_error = PN532_MifareClassicAuthenticateBlock(pn532, uid, uid_len,
				6, MIFARE_CMD_AUTH_B, key);
		if (pn532_error)
			return 1;
	}
	pn532_error = PN532_MifareClassicReadBlock(pn532, data, blk);
	if (pn532_error)
		return 2;
	return 0;
}

/*
uint8_t rfid_mf_wblk(uint8_t *uid, uint8_t blk, uint8_t *key, uint8_t *data)
{
	return 0;
}
*/

uint8_t execute_command(PN532 *pn532)
{
	uint8_t ret;
	static uint8_t uid[MIFARE_UID_MAX_LENGTH];
	static int32_t uid_len = 0;

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);
	switch (data[0])
	{
	case RFID_CMD_READ_BLK:
		ret = rfid_mf_get_uid(pn532, uid, &uid_len);
		if (!ret)
		{
			//ret = rfid_mf_rblk(pn532, uid, uid_len, );
			if (!ret)
				break;
		}
		else
		{
			// No card
		}
		break;
	case RFID_CMD_WRITE_BLK:
		break;
	case RFID_CMD_GET_UID:
	default:
		ret = rfid_mf_get_uid(pn532, uid, &uid_len);
		if (!ret)
		{
			uint8_t yes_card[7] = {0x56, 0x30, uid_len, uid[0], uid[1], uid[2], uid[3]};
			CDC_Transmit_FS(yes_card, 7);	
		}
		else
		{
			uint8_t no_card[3] = {0x56, 0x01, 00};
			CDC_Transmit_FS(no_card, 3);	
		}
		break;
	}
	data_len = 0;
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
	return 0;
}

uint8_t execute_state()
{
	static PN532 pn532;
	static enum rfid_state state = RFID_INIT;

	switch (state)
	{
		case RFID_INIT:
			rfid_init(&pn532);
			state = RFID_IDLE;
			break;
		case RFID_IDLE:
			// Parse if data exists
			if (data_len > 0)
			{
				// cmd = ??
				// 
				state = RFID_EXECUTE;
			}
			break;
		case RFID_EXECUTE:
			execute_command(&pn532);
			state = RFID_IDLE;
			break;
		default:
			break;
	}
	return 0;
}
