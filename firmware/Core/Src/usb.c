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

void send_ping()
{
	uint8_t ping_frame[2] = {0x56, 0x00};
	CDC_Transmit_FS(ping_frame, 2);	
}

void send_uid(uint8_t *uid, uint8_t uid_len)
{
	uint8_t i;
	uint8_t frame[13];

	frame[0] = 0x56;
	frame[1] = 0x01;
	if (!uid || uid_len == 0 ||  uid_len > 10)
	{
		frame[2] = 0x00;
		CDC_Transmit_FS(frame, 3);
		return ;
	}
	frame[2] = uid_len;
	i = 0;
	while (i < uid_len)
	{
		frame[i + 3] = uid[i];
		i++;
	}
	CDC_Transmit_FS(frame, uid_len + 3);
	return ;
}

void send_blk(uint8_t *data)
{
	uint8_t i;
	uint8_t frame[18];

	frame[0] = 0x56;
	if (!data)
	{
		frame[1] = 0x00;
		CDC_Transmit_FS(frame, 3);
		return ;
	}
	frame[1] = 0x02;
	i = 0;
	while (i < 16)
	{
		frame[i + 2] = data[i];
		i++;
	}
	CDC_Transmit_FS(frame, 18);
	return ;
}

void send_wr(uint8_t conf)
{
	uint8_t frame[3];

	frame[0] = 0x56;
	frame[1] = 0x03;
	frame[2] = conf;
	CDC_Transmit_FS(frame, 3);
	return ;
}

void USB_CDC_RxHandler(uint8_t* Buf, uint32_t Len)
{
	if (Len < 2 || Buf[0] != 0x55 || (data_len > 0 && Buf[1] != RFID_CMD_PING))
	{
		send_ping();
		return ;
	}
	switch (Buf[1])
	{
	case RFID_CMD_GET_UID:
		if (Len != 2)
		{
			send_ping();
			break;
		}
		copy_data(Buf + 1, 1);
		break;
	case RFID_CMD_READ_BLK:
		if (Len != 9)
		{
			send_ping();
			break;
		}
		copy_data(Buf + 1, 8);
		break;
	case RFID_CMD_WRITE_BLK:
		if (Len != 25)
		{
			send_ping();
			break;
		}
		copy_data(Buf + 1, 24);
		break;
	case RFID_CMD_PING:
	default:
		send_ping();
		break;
	}
	return ;
}
