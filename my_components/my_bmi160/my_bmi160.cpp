#include "my_bmi160.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::my_bmi160 {
static const char *const TAG = "my_bmi160";

const uint8_t BMI160_REGISTER_ACCEL_CONFIG = 0x40;
enum class AcclFilterMode : uint8_t {
  POWER_SAVING = 0b00000000,
  PERF = 0b10000000,
};
enum class AcclBandwidth : uint8_t {
  OSR4_AVG1 = 0b00000000,
  OSR2_AVG2 = 0b00010000,
  NORMAL_AVG4 = 0b00100000,
  RES_AVG8 = 0b00110000,
  RES_AVG16 = 0b01000000,
  RES_AVG32 = 0b01010000,
  RES_AVG64 = 0b01100000,
  RES_AVG128 = 0b01110000,
};
enum class AccelOutputDataRate : uint8_t {
  HZ_25_32 = 0b0001,  // 25/32 Hz
  HZ_25_16 = 0b0010,  // 25/16 Hz
  HZ_25_8 = 0b0011,   // 25/8 Hz
  HZ_25_4 = 0b0100,   // 25/4 Hz
  HZ_25_2 = 0b0101,   // 25/2 Hz
  HZ_25 = 0b0110,     // 25 Hz
  HZ_50 = 0b0111,     // 50 Hz
  HZ_100 = 0b1000,    // 100 Hz
  HZ_200 = 0b1001,    // 200 Hz
  HZ_400 = 0b1010,    // 400 Hz
  HZ_800 = 0b1011,    // 800 Hz
  HZ_1600 = 0b1100,   // 1600 Hz
};
const uint8_t BMI160_REGISTER_ACCEL_RANGE = 0x41;
enum class AccelRange : uint8_t {
  RANGE_2G = 0b0011,
  RANGE_4G = 0b0101,
  RANGE_8G = 0b1000,
  RANGE_16G = 0b1100,
};

const uint8_t BMI160_REGISTER_GYRO_CONFIG = 0x42;
enum class GyroBandwidth : uint8_t {
  OSR4 = 0x00,
  OSR2 = 0x10,
  NORMAL = 0x20,
};
enum class GyroOuputDataRate : uint8_t {
  HZ_25 = 0x06,
  HZ_50 = 0x07,
  HZ_100 = 0x08,
  HZ_200 = 0x09,
  HZ_400 = 0x0A,
  HZ_800 = 0x0B,
  HZ_1600 = 0x0C,
  HZ_3200 = 0x0D,
};
const uint8_t BMI160_REGISTER_GYRO_RANGE = 0x43;
enum class GyroRange : uint8_t {
  RANGE_2000_DPS = 0x0,  // ±2000 °/s
  RANGE_1000_DPS = 0x1,
  RANGE_500_DPS = 0x2,
  RANGE_250_DPS = 0x3,
  RANGE_125_DPS = 0x4,
};

const uint8_t BMI160_REGISTER_DATA_GYRO_X_LSB = 0x0C;
const uint8_t BMI160_REGISTER_DATA_GYRO_X_MSB = 0x0D;
const uint8_t BMI160_REGISTER_DATA_GYRO_Y_LSB = 0x0E;
const uint8_t BMI160_REGISTER_DATA_GYRO_Y_MSB = 0x0F;
const uint8_t BMI160_REGISTER_DATA_GYRO_Z_LSB = 0x10;
const uint8_t BMI160_REGISTER_DATA_GYRO_Z_MSB = 0x11;
const uint8_t BMI160_REGISTER_DATA_ACCEL_X_LSB = 0x12;
const uint8_t BMI160_REGISTER_DATA_ACCEL_X_MSB = 0x13;
const uint8_t BMI160_REGISTER_DATA_ACCEL_Y_LSB = 0x14;
const uint8_t BMI160_REGISTER_DATA_ACCEL_Y_MSB = 0x15;
const uint8_t BMI160_REGISTER_DATA_ACCEL_Z_LSB = 0x16;
const uint8_t BMI160_REGISTER_DATA_ACCEL_Z_MSB = 0x17;
const uint8_t BMI160_REGISTER_DATA_TEMP_LSB = 0x20;
const uint8_t BMI160_REGISTER_DATA_TEMP_MSB = 0x21;


void MyBMI160Component::internal_setup_(int stage) {
  switch (stage) {
    case 0:
      bmi160::BMI160Component::internal_setup_(stage);
      break;
    case 1:
      bmi160::BMI160Component::internal_setup_(stage);
      break;

    case 2:
      ESP_LOGV(TAG, "  Setting up Gyro Config");
      uint8_t gyro_config = (uint8_t) GyroBandwidth::OSR4 | (uint8_t) GyroOuputDataRate::HZ_50;
      ESP_LOGV(TAG, "  Output gyro_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(gyro_config));
      if (!this->write_byte(BMI160_REGISTER_GYRO_CONFIG, gyro_config)) {
        this->mark_failed();
        return;
      }
      ESP_LOGV(TAG, "  Setting up Gyro Range");
      uint8_t gyro_range = (uint8_t) GyroRange::RANGE_2000_DPS;
      ESP_LOGV(TAG, "  Output gyro_range: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(gyro_range));
      if (!this->write_byte(BMI160_REGISTER_GYRO_RANGE, gyro_range)) {
        this->mark_failed();
        return;
      }

      ESP_LOGV(TAG, "  Setting up Accel Config");
      uint8_t accel_config =
          (uint8_t) AcclFilterMode::PERF | (uint8_t) AcclBandwidth::RES_AVG16 | (uint8_t) AccelOutputDataRate::HZ_50;
      ESP_LOGV(TAG, "  Output accel_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(accel_config));
      if (!this->write_byte(BMI160_REGISTER_ACCEL_CONFIG, accel_config)) {
        this->mark_failed();
        return;
      }
      ESP_LOGV(TAG, "  Setting up Accel Range");
      uint8_t accel_range = (uint8_t) AccelRange::RANGE_16G;
      ESP_LOGV(TAG, "  Output accel_range: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(accel_range));
      if (!this->write_byte(BMI160_REGISTER_ACCEL_RANGE, accel_range)) {
        this->mark_failed();
        return;
      }

      this->setup_complete_ = true;
  }
}

}  // namespace esphome::my_bmi160
