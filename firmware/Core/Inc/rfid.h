#ifndef RFID_H
#define RFID_H

#include <stdint.h>

#define MAX_USB_FRAME_LENGTH 24

enum rfid_state {
	RFID_INIT = 0x00,
	RFID_IDLE,
	RFID_EXECUTE,
};

enum rfid_command {
	RFID_CMD_PING = 0x00,
	RFID_CMD_GET_UID,
	RFID_CMD_READ_BLK,
	RFID_CMD_WRITE_BLK,
};

extern uint8_t data[MAX_USB_FRAME_LENGTH];
extern uint8_t data_len;

uint8_t execute_state();
#endif // RFID_H
