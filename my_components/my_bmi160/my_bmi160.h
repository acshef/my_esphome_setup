#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/bmi160/bmi160.h"

namespace esphome::my_bmi160 {

class MyBMI160Component : public bmi160::BMI160Component {
  void internal_setup_(int stage);
};

}  // namespace esphome::my_bmi160