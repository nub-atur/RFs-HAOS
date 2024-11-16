/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#define DEBUG_ka

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <ArduinoJson.h>
#include <SPI.h>             
#include <LoRa.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCREEN_WIDTH   128  // OLED display width, in pixels
#define SCREEN_HEIGHT  64  // OLED display height, in pixel
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
#define NO_ACK         0x00
#define YES_ACK        0x01
#define CLEAR(s)       memset(&(s), '\0', sizeof(s))
#define BATTERY_PIN    32
#define LED          25

#define   MESH_PREFIX     "RNTMESH"
#define   MESH_PASSWORD   "MESHpassword"
#define   MESH_PORT       5555

#define   ss              5
#define   rst             14
#define   dio0            13

struct data
{
  /* data */
  uint8_t ack_get2, ack_get1;
  double temp1, hum1, motor2, obs2;
  int time1, time2;
};

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */