#ifndef USB_H
#define USB_H

#include "stm32f0xx_hal.h"
#include "usbd_cdc_if.h"
#include "rfid.h"

void USB_CDC_RxHandler(uint8_t*, uint32_t);

#endif // USB_H
