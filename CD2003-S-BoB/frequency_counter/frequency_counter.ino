// Runs on Raspberry Pi Pico W / Pico

#include <Wire.h>
#include <Arduino.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/irq.h"   // interrupts
#include "hardware/pwm.h"   // pwm
#include "hardware/sync.h"  // wait for interrupt

#define FREQUENCY_INPUT_PIN 15

// Automatic calibration
uint32_t f_hi;

void pwm_int() {
  pwm_clear_irq(7);
  f_hi++;
}

void do_calibration() {
  int64_t existing_error = 0;
  int64_t error = 0;
  int count = 3;                         // reverse count
  uint64_t target_freq = 1000000000ULL;  // 10 MHz, in hundredths of hertz

  uint32_t f = 0;
  // Frequency counter
  while (true) {
    count = count - 1;
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
    pwm_init(7, &cfg, false);
    gpio_set_function(FREQUENCY_INPUT_PIN, GPIO_FUNC_PWM);
    pwm_set_irq_enabled(7, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_int);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    f_hi = 0;
    uint32_t t = time_us_32() + 2;
    while (t > time_us_32())
      ;
    pwm_set_enabled(7, true);
    t += 3000000;  // Gate time (in uSeconds), 3 seconds
    // t += 100000; // Gate time (in uSeconds), 100 ms
    while (t > time_us_32())
      ;
    pwm_set_enabled(7, false);
    f = pwm_get_counter(7);
    f += f_hi << 16;
    Serial.print(f / 3.0);  // Divide by gate time in seconds
    Serial.println(" Hz");
  }
}

void setup() {
  int ret = 0;

  // Setup I/O pins
  gpio_set_function(FREQUENCY_INPUT_PIN, GPIO_FUNC_PWM);
  Serial.begin(115200);
  delay(5000);

  // Perform automatic calibration
  while (1)
      do_calibration();
}

void loop() {
  delay(10);
}
