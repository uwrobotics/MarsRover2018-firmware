/*
 * pins.h
 *
 *  Created on: Jan. 30, 2017
 *      Author: Archie Lee
 *
 *  Description:
 *  Defines common pin IDs and ports
 */

#ifndef PINS_H_
#define PINS_H_

#define DISCOVERY       1

#include "stm32f0xx.h"

// LEDs for discovery board
#if DISCOVERY
#define LED1_GPIO_PIN       GPIO_PIN_9
#define LED2_GPIO_PIN       GPIO_PIN_8
#define LED3_GPIO_PIN       GPIO_PIN_7
#define LED4_GPIO_PIN       GPIO_PIN_6
#define LED_GPIO_PORT       GPIOC
#else
#define LED1_GPIO_PIN       GPIO_PIN_11
#define LED2_GPIO_PIN       GPIO_PIN_10
#define LED_GPIO_PORT       GPIOC
#endif

// CAN
#define CAN_GPIO_RX_PIN     GPIO_PIN_8
#define CAN_GPIO_TX_PIN     GPIO_PIN_9
#define CAN_GPIO_PORT       GPIOB

// PWM
#define PWM1_GPIO_PIN       GPIO_PIN_8
#define PWM1_GPIO_PORT      GPIOA
#define PWM2_GPIO_PIN       GPIO_PIN_9
#define PWM2_GPIO_PORT      GPIOA
#define PWM3_GPIO_PIN       GPIO_PIN_10
#define PWM3_GPIO_PORT      GPIOA

// Encoders
#define ENCODER_1A_PIN      GPIO_PIN_15
#define ENCODER_1A_PORT     GPIOA
#define ENCODER_1B_PIN      GPIO_PIN_3
#define ENCODER_1B_PORT     GPIOB
#define ENCODER_2A_PIN      GPIO_PIN_6
#define ENCODER_2A_PORT     GPIOC
#define ENCODER_2B_PIN      GPIO_PIN_7
#define ENCODER_2B_PORT     GPIOC

// I2C
#define I2C1_SCL_GPIO_PIN GPIO_PIN_6
#define I2C1_SDA_GPIO_PIN GPIO_PIN_7
#define I2C1_GPIO_PORT GPIOB
// I2C
#define I2C2_SCL_GPIO_PIN GPIO_PIN_10
#define I2C2_SDA_GPIO_PIN GPIO_PIN_11
#define I2C2_GPIO_PORT GPIOB

#endif
