#ifndef DDX_common
#define DDX_common

#define VERSION        "3.0"
#define BUILD          1

#include <stdint.h>
#include <si5351.h>

#include "hardware/watchdog.h"
#include "Wire.h"
// #include <EEPROM.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include <stdio.h>
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/uart.h"

// Pinout
#define PTT            14 // PTT pin
#define CAL            15 // Automatic calibration entry
#define TXLED           5
#define DEBUG           8
#define GPS             9
#define SDA_PIN        12
#define SCL_PIN        13
#define RESET_PIN      10 // Si4732 stuff
#define ATU_PIN        0
// Buttons
#define CALIBRATE_BTN  1
#define ABORT_BTN      6
#define TUNE_BTN       7

extern unsigned long  freq;
extern uint16_t       mode;
extern int32_t        cal_factor;
extern Si5351         si5351;

void serialEvent();
void led_flash();

#define AM_FUNCTION 1
#define LSB 1
#define USB 2

#endif
