#ifndef USB_H
#define USB_H

#include "stm32f0xx_hal.h"
#include "usbd_cdc_if.h"
#include "rfid.h"

void USB_CDC_RxHandler(uint8_t*, uint32_t);

void send_ping();
void send_uid(uint8_t *uid, uint8_t uid_len);
void send_blk(uint8_t *data);
void send_wr(uint8_t conf);

#endif // USB_H
