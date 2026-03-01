#include "usb.h"

void copy_data(uint8_t *src, uint8_t len)
{
	uint8_t i;

	i = 0;
	while (i < MAX_USB_FRAME_LENGTH)
	{
		if (i < len)
			data[i] = src[i];
		else
			data[i] = 0;	
		i++;
	}
	data_len = len;
	return ;
}

void USB_CDC_RxHandler(uint8_t* Buf, uint32_t Len)
{
	uint8_t ping_frame[2] = {0x56, 0x00};

	if (Len < 2 || Buf[0] != 0x55 || (data_len > 0 && Buf[1] != RFID_CMD_PING))
	{
		CDC_Transmit_FS(ping_frame, 2);
		return ;
	}
	switch (Buf[1])
	{
	case RFID_CMD_GET_UID:
		if (Len != 2)
		{
			CDC_Transmit_FS(ping_frame, 2);	
			break;
		}
		copy_data(Buf + 1, 1);
		break;
	case RFID_CMD_READ_BLK:
		if (Len != 8)
		{
			CDC_Transmit_FS(ping_frame, 2);
			break;
		}
		copy_data(Buf + 1, 7);
		break;
	case RFID_CMD_WRITE_BLK:
		if (Len != 24)
		{
			CDC_Transmit_FS(ping_frame, 2);
			break;
		}
		copy_data(Buf + 1, 23);
		break;
	case RFID_CMD_PING:
	default:
		CDC_Transmit_FS(ping_frame, 2);
		break;
	}
	return ;
}
