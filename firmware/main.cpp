#include <string>
#include <ostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
// Uncomment if overclocking > 290MHz
#include "hardware/vreg.h"

#include "Wire.h"

#include "hardware/adc.h"
#include "hardware/dma.h"

// Si4732
#include "SI4735.h"
#include "patch_full.h"    // SSB patch for whole SSBRX full download

#include "ddx_common.h"

// SI5351
#include <si5351.h>
Si5351 si5351;

// ft8_lib stuff
#include "ft8/pack.h"
#include "ft8/encode.h"
#include "ft8/decode_ft8.h"
#include "ft8/gen_ft8.h"

#include "util/rx_ft8.h"

#include <TimeLib.h>

const uint LED_PIN = 25;

message_info message_list[kMax_decoded_messages]; // probably needs to be memset cleared before each decode

int16_t signal_for_processing[num_samples_processed] = {0};

uint32_t handler_max_time = 0;

void core1_irq_handler()
{
  // Copy capture_buf to signal_for_processing array, take fft and save to power
  while (multicore_fifo_rvalid())
  {
    uint32_t handler_start = time_us_32();
    uint16_t idx = multicore_fifo_pop_blocking();

    for (int i = 0; i < nfft; i++) {
      fresh_signal[i] -= DC_BIAS;
    }
    inc_extract_power(fresh_signal);
    uint32_t handler_time = (time_us_32() - handler_start) / 1000;
    if (handler_time > handler_max_time) {
      handler_max_time = handler_time;
    }
    // handler MUST BE under 160 ms.
  }

  multicore_fifo_clear_irq(); // Clear interrupt
}

void core1_runner(void)
{
  // Configure Core 1 Interrupt
  printf("second core running!\n");
  multicore_fifo_clear_irq();
  irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_irq_handler);

  irq_set_enabled(SIO_IRQ_PROC1, true);

  // Infinite While Loop to wait for interrupt
  while (1)
  {
    tight_loop_contents();
  }
}

// Automatic calibration stuff
uint32_t f_hi_;
void pwm_int_() {
  pwm_clear_irq(7);
  f_hi_++;
}

// Variables
int pwm_slice;
uint32_t ffsk     = 0;
unsigned long freq = 14074000; // Start on the "DX band (20m)" by default
unsigned long new_freq;
int32_t cal_factor = 116566; // Sorted out by auto-calibration algorithm

void do_calibration()
{
  gpio_set_function(CAL, GPIO_FUNC_PWM);

  // Set CLK0 output for calibration
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA); // Set for minimum power as we have an external amplifier
  si5351.set_freq(10000000 * 100ULL, SI5351_CLK2); // used for calibration
  si5351.output_enable(SI5351_CLK2, 1);
  // Warmup
  Serial.println("Warming up for 7 seconds...");
  Serial.println("Warming up for 7 seconds...");
  Serial.println("Warming up for 7 seconds...");
  Serial.println("Warming up for 7 seconds...");
  Serial.println("Warming up for 7 seconds...");
  Serial.println("Warming up for 7 seconds...");
  delay(7000);
  // Frequency counter
  int64_t existing_error = 0;
  int64_t error = 0;
  int count = 3; // reverse count
  uint64_t target_freq = 1000000000ULL; // 10 MHz, in hundredths of hertz
  uint32_t f = 0;
  while (true) {
    count = count - 1;
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
    pwm_init(7, &cfg, false);
    gpio_set_function(CAL, GPIO_FUNC_PWM);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_int_);
    pwm_set_irq_enabled(7, true);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    f_hi_ = 0;
    si5351.set_correction(0, SI5351_PLL_INPUT_XO); // important - reset calibration
    uint32_t t = time_us_32() + 2;
    while (t > time_us_32());
    pwm_set_enabled(7, true);
    t += 3000000; // Gate time (in uSeconds), 3 seconds
    // t += 100000; // Gate time (in uSeconds), 100 ms
    while (t > time_us_32());
    pwm_set_enabled(7, false);
    f = pwm_get_counter(7);
    f += f_hi_ << 16;
    Serial.print(f / 3.0); // Divide by gate time in seconds
    Serial.println(" Hz");
    error = ((f / 3.0) * 100ULL) - target_freq;
    Serial.print("Current calibration correction value is ");
    Serial.print("Total calibration value is ");
    Serial.println(error + existing_error);
    if (count <= 0) { // Auto-calibration logic
      Serial.println();
      Serial.print(F("Calibration value is "));
      Serial.println(error);
      Serial.println(F("Setting calibration value automatically"));
      si5351.set_correction(error + existing_error, SI5351_PLL_INPUT_XO);
      existing_error = existing_error + error;
      si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
      Serial.println(F("Resetting target frequency"));
      si5351.set_freq(target_freq, SI5351_CLK0);
      // count = 3;
      break;
    }
  }

  // Save to EEPROM
  // uint32_t magic = 0x1CEB00DA;
  // EEPROM.put(0, magic);
  // EEPROM.put(4, (uint32_t)existing_error);
  // EEPROM.commit();
  // delay(7000);
  // Serial.printf("Saving calibration factor (%d) to EEPROM\n", existing_error);
}

const uint16_t size_content = sizeof ssb_patch_content; // see ssb_patch_content in patch_full.h or patch_init.h
bool disableAgc = true;
bool avc_en = true;
int currentBFO = 0;
long et1 = 0, et2 = 0;

// Some variables to check the SI4735 status
uint16_t currentFrequency;
uint8_t currentStep = 1;
uint8_t currentBFOStep = 25;

uint8_t bandwidthIdx = 2;
const char *bandwidth[] = {"1.2", "2.2", "3.0", "4.0", "0.5", "1.0"};

typedef struct
{
  uint16_t minimumFreq;
  uint16_t maximumFreq;
  uint16_t currentFreq;
  uint16_t currentStep;
  uint8_t currentSSB;
} Band;

Band band[] = {
  {14000, 14300, 14074, 1, USB},
  {21000, 21300, 21074, 1, USB},
  {28000, 28300, 28074, 1, USB},
};

int currentFreqIdx = 0; // 20m band

SI4735 si4735;

void band_change(int idx)
{
  si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
  si4735.setSSB(band[idx].minimumFreq, band[idx].maximumFreq, band[idx].currentFreq, band[idx].currentStep, band[idx].currentSSB);
  currentStep = band[idx].currentStep;
  delay(250);
  currentFrequency = si4735.getCurrentFrequency();
}

void loadSSB()
{
  si4735.setI2CFastModeCustom(500000);
  si4735.queryLibraryId(); // Is it really necessary here? I will check it.
  si4735.patchPowerUp();
  delay(50);
  si4735.downloadPatch(ssb_patch_content, size_content);
  // Parameters
  // AUDIOBW - SSB Audio bandwidth; 0 = 1.2kHz (default); 1=2.2kHz; 2=3kHz; 3=4kHz; 4=500Hz; 5=1kHz;
  // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
  // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
  // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
  // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
  // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
  si4735.setSSBConfig(bandwidthIdx, 1, 0, 1, 0, 1);
  si4735.setI2CFastModeCustom(100000);
}

/* Button handling code */
int btnDown(int btn) {
  if (digitalRead(btn) == HIGH)
    return 0;
  else
    return 1;
}

void active_delay(unsigned int delay_by) {
  unsigned long timeStart = millis();

  while (millis() - timeStart <= delay_by) {
    // Background Work
  }
}

int checkButton(int btn) {
  int ret = 0;

  // only if the button is pressed
  if (!btnDown(btn))
    return ret;
  active_delay(50);
  if (!btnDown(btn)) // debounce
    return ret;

  ret = 1;
  if (btn == ABORT_BTN) {
    digitalWrite(PTT, LOW);
  } else if (btn == CALIBRATE_BTN) {
    // TODO
  } else if (btn == TUNE_BTN) {
    digitalWrite(ATU_PIN, 1);
    delay(3500); // Full tune cycle
    digitalWrite(ATU_PIN, 0);
  }

  // wait for the button to go up again
  while (btnDown(btn))
    active_delay(10);
  active_delay(50);

  return ret;
}

int main(void)
{
  // Overclocking the processor
  // 133 MHz is the default, 250 MHz is safe at 1.1V and for flash
  // If using clock > 290MHz, increase voltage and add flash divider
  // See https://raspberrypi.github.io/pico-sdk-doxygen/vreg_8h.html
  // vreg_set_voltage(VREG_VOLTAGE_DEFAULT); // default: VREG_VOLTAGE_1_10 max:VREG_VOLTAGE_1_30
  vreg_set_voltage(VREG_VOLTAGE_1_30); // default: VREG_VOLTAGE_1_10 max:VREG_VOLTAGE_1_30
  // set_sys_clock_khz(250000, true);
  // set_sys_clock_khz(250000, true);
  set_sys_clock_khz(290400, true);
  setup_default_uart();

  int ret;

  // Initialize GPIOs
  gpio_init(LED_PIN);
  gpio_init(PTT);
  gpio_init(TUNE_BTN);
  gpio_init(CALIBRATE_BTN);
  gpio_init(ABORT_BTN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_set_dir(PTT, GPIO_OUT);
  gpio_set_dir(TUNE_BTN, GPIO_OUT);
  gpio_set_dir(CALIBRATE_BTN, GPIO_OUT);
  gpio_set_dir(ABORT_BTN, GPIO_OUT);
  gpio_pull_up(TUNE_BTN);
  gpio_pull_up(CALIBRATE_BTN);
  gpio_pull_up(ABORT_BTN);
  pinMode(CAL, INPUT);
  gpio_put(PTT, 0);
  gpio_put(ATU_PIN, 0);

  // setup the adc
  setup_adc();

  stdio_init_all();

  Serial.begin(115200);

  delay(7000);

  // "Pico's" serial number
  int len = 16;
  char buff[len] = "";
  pico_get_unique_board_id_string((char *)buff, len);
  Serial.println(buff); // works till here

  // I2C pins
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  // Si5351 stuff
  ret = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, cal_factor);
  if (ret != true) {
    led_flash();
    watchdog_reboot(0, 0, 1000);
  }
  si5351.output_enable(SI5351_CLK0, 0); // Turn of TX first - safety!
  // Note: Enable this in production!
  // do_calibration();
  // Turn on RX
  digitalWrite(PTT, 0);
  // EEPROM.get(4, cal_factor);
  // si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA); // Set for reduced power for RX
  si5351.output_enable(SI5351_CLK0, 0); // Safety first
  si5351.set_freq(3276800, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 1);

  // Si4732 stuff
  int16_t si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
  if (si4735Addr == 0) {
    Serial.println("Si473X not found!");
    Serial.flush();
    // while (1);
  } else {
    Serial.print("The Si473X I2C address is 0x");
    Serial.println(si4735Addr, HEX);
  }

  si4735.setup(RESET_PIN, AM_FUNCTION);
  delay(10);
  Serial.println("SSB patch is loading...");
  et1 = millis();
  loadSSB();
  et2 = millis();
  Serial.print("SSB patch was loaded in: ");
  Serial.print( (et2 - et1) );
  Serial.println("ms");
  delay(100);
  si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
  si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentFreqIdx].currentSSB);
  delay(100);
  currentFrequency = si4735.getFrequency();
  si4735.setAvcAmMaxGain(60); // Sets the maximum gain for automatic volume control on AM/SSB mode (from 12 to 90dB)
  si4735.setVolume(60);

  // Make hanning window for fft work
  make_window();

  // Start //
  char message[32];
  uint8_t tones[256];
  int offset = 0;
  unsigned char rec_byte[2];
  unsigned int msg_index;
  bool message_available = true;
  bool send = false;
  bool justSent = false; // must recieve right after sending
  bool autosend = true;
  bool cq_only = false;

  // Sync time
  // sync_time_with_gps_with_timeout();

  // launch second core
  multicore_launch_core1(core1_runner);

  int fudge = 0;
  int do_fudging = 1;

  // decode loop
  while (true) {
    if ((second() + fudge) % 15 == 0) { // RX
      printf("RECEIVING FOR 12.8 SECONDS. Second -> %d Fudge -> %d\n\n", second(), fudge);
      uint32_t start = time_us_32();
      inc_collect_power();
      uint32_t stop = time_us_32();
      printf("Handler max time: %d\n", handler_max_time);
      printf("Recording time: %d us\n", stop - start);
      uint32_t decode_begin = time_us_32();
      uint8_t num_decoded = decode_ft8(message_list);
      printf("Decoding took this long: %d us\n", time_us_32() - decode_begin);
      decode_begin = time_us_32();
      if (num_decoded > 0) {
        do_fudging = 0;
        printf("FINAL FUDGE -> %d\n", fudge);
      }
      if (do_fudging)
        fudge = (fudge + 1) % 15;
    }
    delay(10);
  }

  return 0;
}

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
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
}
