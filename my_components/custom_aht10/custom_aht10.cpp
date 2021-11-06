/*
  This is an Arduino library for Aosong ASAIR AHT10, AHT15 Digital Humidity & Temperature Sensor

  written by : enjoyneering79
  source code: https://github.com/enjoyneering/


  This chip uses I2C bus to communicate, specials pins are required to interface
  Board:                                    SDA                    SCL                    Level
  Uno, Mini, Pro, ATmega168, ATmega328..... A4                     A5                     5v
  Mega2560................................. 20                     21                     5v
  Due, SAM3X8E............................. 20                     21                     3.3v
  Leonardo, Micro, ATmega32U4.............. 2                      3                      5v
  Digistump, Trinket, ATtiny85............. 0/physical pin no.5    2/physical pin no.7    5v
  Blue Pill, STM32F103xxxx boards.......... PB7                    PB6                    3.3v/5v
  ESP8266 ESP-01........................... GPIO0/D5               GPIO2/D3               3.3v/5v
  NodeMCU 1.0, WeMos D1 Mini............... GPIO4/D2               GPIO5/D1               3.3v/5v
  ESP32.................................... GPIO21/D21             GPIO22/D22             3.3v

  Frameworks & Libraries:
  ATtiny  Core          - https://github.com/SpenceKonde/ATTinyCore
  ESP32   Core          - https://github.com/espressif/arduino-esp32
  ESP8266 Core          - https://github.com/esp8266/Arduino
  STM32   Core          - https://github.com/stm32duino/Arduino_Core_STM32
                        - https://github.com/rogerclarkmelbourne/Arduino_STM32

  GNU GPL license, all text above must be included in any redistribution,
  see link for details  - https://www.gnu.org/licenses/licenses.html
*/

#include "custom_aht10.h"
#include "esphome/core/log.h"

namespace esphome {
namespace custom_aht10 {

static const char *TAG = "custom_aht10";
// static const uint8_t AHT10_CALIBRATE_CMD[] = {0xE1};
// static const uint8_t AHT10_MEASURE_CMD[] = {0xAC, 0x33, 0x00};
// static const uint8_t AHT10_DEFAULT_DELAY = 5;    // ms, for calibration and temperature measurement
// static const uint8_t AHT10_HUMIDITY_DELAY = 30;  // ms
static const uint8_t  AHT10_ATTEMPTS          = 3;                  // Safety margin, normally 3 attemps are enough: 3*30=90ms
static const uint8_t  AHT10_NORMAL_MODE_CMD[] = {0xA8, 0x00, 0x00}; // Normal cycle mode command, no info in datasheet!!!
static const uint8_t  AHT10_CALIBRATE_CMD[]   = {0xE1, 0x08, 0x00}; // Load factory calibration coeff
static const uint8_t  AHT10_MEASURE_CMD[]     = {0xAC, 0x33, 0x00};

static const uint8_t  AHT10_POWER_ON_DELAY    = 40;                 // At least 20-40 milliseconds
static const uint32_t AHT10_CMD_DELAY         = 350;                // At least 300 milliseconds, no info in datasheet!!!
static const uint8_t  AHT10_DEFAULT_DELAY     = 5;
static const uint8_t  AHT10_MEASUREMENT_DELAY = 80;                 // At least 75 milliseconds

void CustomAHT10Component::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Custom AHT10...");

    delay(AHT10_POWER_ON_DELAY);

    if (!this->set_normal_mode()) {
        ESP_LOGE(TAG, "Initialization of AHT10 failed!");
        this->mark_failed();
        return;
    }

    if (!this->enable_factory_cal_coeff()) {
        ESP_LOGE(TAG, "Calibration of AHT10 failed!");
        this->mark_failed();
        return;
    }

    ESP_LOGV(TAG, "AHT10 calibrated");
}

/*
  Set normal measurement mode
  a.k.a one measurement and then power down??? there's no info in the datasheet about this!!!
*/
bool CustomAHT10Component::set_normal_mode() {
    bool normal_cmd_response = this->write_bytes(0, AHT10_NORMAL_MODE_CMD, sizeof(AHT10_NORMAL_MODE_CMD));
    delay(AHT10_CMD_DELAY);
    return normal_cmd_response;
}

/*
  Load factory calibration coefficients
*/
bool CustomAHT10Component::enable_factory_cal_coeff() {
    bool calibration_response = this->write_bytes(0, AHT10_CALIBRATE_CMD, sizeof(AHT10_CALIBRATE_CMD));
    delay(AHT10_CMD_DELAY);
    if (!calibration_response) {
        return false;
    }

    uint8_t data = 0;

    delay(AHT10_DEFAULT_DELAY);
    if (this->read(&data, 1) != i2c::ERROR_OK) {
      ESP_LOGVV(TAG, "Couldn't read data for determining calibration");
      return false;
    }

    if ((data & 0x68) != 0x08) { // Bit[6:5]
      ESP_LOGVV(TAG, "AHT10 not: in normal mode and calibrated");
      return false;
    }

    return true;
}

void CustomAHT10Component::update() {
  if (!this->write_bytes(0, AHT10_MEASURE_CMD, sizeof(AHT10_MEASURE_CMD))) {
    ESP_LOGE(TAG, "Communication with AHT10 failed!");
    this->status_set_warning();
    return;
  }

  bool success = false;
  uint8_t data[6];

  for (int i = 0; i < AHT10_ATTEMPTS; ++i) {
    delay(AHT10_MEASUREMENT_DELAY);
    ESP_LOGVV(TAG, "Attempt %u at %6ld", i, millis());
    if (this->read(data, 6) != i2c::ERROR_OK) {
      ESP_LOGD(TAG, "Communication with AHT10 failed, waiting...");
      continue;
    }

    if ((data[0] & 0x80) == 0x80) {  // Bit[7] = 0b1, device is busy
      ESP_LOGD(TAG, "AHT10 is busy, waiting...");
    } else if (data[1] == 0x0 && data[2] == 0x0 && (data[3] >> 4) == 0x0) {
      // Unrealistic humidity (0x0)
      if (this->humidity_sensor_ == nullptr) {
        ESP_LOGVV(TAG, "ATH10 Unrealistic humidity (0x0), but humidity is not required");
        break;
      } else {
        ESP_LOGD(TAG, "ATH10 Unrealistic humidity (0x0), retrying...");
        if (!this->write_bytes(0, AHT10_MEASURE_CMD, sizeof(AHT10_MEASURE_CMD))) {
          ESP_LOGE(TAG, "Communication with AHT10 failed!");
          this->status_set_warning();
          return;
        }
      }
    } else {
      // data is valid, we can break the loop
      ESP_LOGVV(TAG, "Answer at %6ld", millis());
      success = true;
      break;
    }
  }

  if (!success || (data[0] & 0x80) == 0x80) {
    ESP_LOGE(TAG, "Measurements reading timed-out!");
    this->status_set_warning();
    return;
  }

  uint32_t raw_temperature = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];
  uint32_t raw_humidity = ((data[1] << 16) | (data[2] << 8) | data[3]) >> 4;

  float temperature = ((200.0 * (float) raw_temperature) / 1048576.0) - 50.0;
  float humidity;
  if (raw_humidity == 0) {  // unrealistic value
    humidity = NAN;
  } else {
    humidity = (float) raw_humidity * 100.0 / 1048576.0;
  }

  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state(temperature);
  }
  if (this->humidity_sensor_ != nullptr) {
    if (isnan(humidity)) {
      ESP_LOGW(TAG, "Invalid humidity! Sensor reported 0%% Hum");
    }
    this->humidity_sensor_->publish_state(humidity);
  }
  this->status_clear_warning();
}

float CustomAHT10Component::get_setup_priority() const { return setup_priority::DATA; }

void CustomAHT10Component::dump_config() {
  ESP_LOGCONFIG(TAG, "CUSTOM AHT10:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with AHT10 failed!");
  }
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
}

}  // namespace custom_aht10
}  // namespace esphome