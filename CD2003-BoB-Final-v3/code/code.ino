// ADX-S on Pico RX code (borrowed from ADX and ADX-S upstream sources)

#include <Wire.h>
#include "si5351.h"

unsigned long long freq;
unsigned long ifreq; // IF frequency, defaults to 464570Hz or 447430Hz for the MURATA PFB455JR ceramic filter
int32_t si5351CalibrationFactor = 0; // We are using a TCXO ;) - https://github.com/kholia/Si5351-Module-Clone-TCXO/

#define SI5351_REF 26000000UL

Si5351 si5351;

// Debug helper
void led_flash()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup()
{
  int ret;

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  // I2C pins
  Wire.setSDA(12);
  Wire.setSCL(13);
  Wire.begin();

  // Initialize the Si5351
  ret = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 26000000, si5351CalibrationFactor);
  if (ret != true) {
    led_flash();
    watchdog_reboot(0, 0, 1000);
  }
  si5351.set_clock_pwr(SI5351_CLK0, 0); // safety first

  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA); // Set for reduced power for RX
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);

  // 20m
  int F_FT8 = 14074000;
  // int F_WSPR = 14095600;
  ifreq = 464570UL;
  // 15m
  // F_FT8 = 21074000;
  // F_WSPR = 21094600;
  // ifreq = 464570UL;

  // 10m
  // F_FT8 = 28074000;
  // F_WSPR = 28124600;
  // ifreq = 464570UL;

  si5351.set_freq(28074000ULL * 100ULL, SI5351_CLK1);
  si5351.output_enable(SI5351_CLK0, 0); // TX off
  si5351.output_enable(SI5351_CLK1, 1); // RX on
  si5351.output_enable(SI5351_CLK2, 0); // RX IF on
}

void loop()
{
 
  delay(10);
}
