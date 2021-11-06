#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace two_pin_garage {
  enum CoverState {
    ERROR = -1,
    UNKNOWN = 0,
    CLOSED = 1,
    OPENING = 2,
    OPEN = 3,
    CLOSING = 4
  };

class TwoPinGarageSensor : public Component, public text_sensor::TextSensor {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  bool door_is_open();
  bool door_is_closed();

  void set_top_pin(GPIOPin *top_pin) { top_pin_ = top_pin; }
  void set_bottom_pin(GPIOPin *bottom_pin) { bottom_pin_ = bottom_pin; }
  void set_debounce_duration(uint32_t debounce) { debounce_ = debounce; }

 protected:
  GPIOPin *top_pin_;
  GPIOPin *bottom_pin_;
  uint32_t debounce_;
  CoverState _cover_state;

  static std::string state_string(CoverState enum_state) {
    switch (enum_state) {
        case CoverState::ERROR:   return "Error";
        case CoverState::CLOSED:  return "Closed";
        case CoverState::OPENING: return "Opening";
        case CoverState::OPEN:    return "Open";
        case CoverState::CLOSING: return "Closing";
        default: break;
      }
      return "Unknown";
  }
};

}  // namespace two_pin_garage
}  // namespace esphome