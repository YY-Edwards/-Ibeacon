/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef PCA20006_H
#define PCA20006_H
 
#include "nrf_gpio.h"
 
#define LED_RED   1
#define LED_GREEN  2
#define LED_BLUE  3

#define LED1   18
#define LED2   19
#define LED3   20

#define BUTTON_0       16
#define BUTTON_1       17
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define RX_PIN_NUMBER  24
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 21
#define RTS_PIN_NUMBER 11
#define HWFC           true

#endif
