#include "two_pin_garage_text_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace two_pin_garage {

static const char *TAG = "two_pin_garage";

void TwoPinGarageSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Two-Pin Garage Cover...");
  top_pin_->setup();
  bottom_pin_->setup();
  _cover_state = CoverState::UNKNOWN;
}

void TwoPinGarageSensor::loop() {
  bool door_open = door_is_open();
  bool door_closed = door_is_closed();
  CoverState old_state = _cover_state;

  if (door_open && !door_closed) {
    _cover_state = CoverState::OPEN;
  } else if (door_closed && !door_open) {
    _cover_state = CoverState::CLOSED;
  } else if (door_closed && door_open) {
    // Door can't be open and closed at the same time
    _cover_state = CoverState::ERROR;
  } else {
    // The door is neither open nor closed; door is somewhere in the middle
    switch (_cover_state) {
      case CoverState::CLOSED:
        // Door was closed, so now it's opening
        _cover_state = CoverState::OPENING;
        break;
      case CoverState::OPEN:
        // Door was open, so now it's closing
        _cover_state = CoverState::CLOSING;
        break;
      default:
        break; // Assume the same as the previous state
    }
  }

  if (old_state != _cover_state) {
    this->publish_state(state_string(_cover_state));
  }
}

bool TwoPinGarageSensor::door_is_open() {
  static bool last_state = top_pin_->digital_read();
  static bool state = last_state;
  static uint32_t last_changed = 0;
  bool reading = top_pin_->digital_read();
  uint32_t now = millis();

  // If value changed, whether noise or contact
  if (reading != last_state) {
    last_changed = now;
  }

  if ((now - last_changed) > this->debounce_) {
    state = reading;
  }

  last_state = reading;
  return state;
}

bool TwoPinGarageSensor::door_is_closed() {
  static bool last_state = bottom_pin_->digital_read();
  static bool state = last_state;
  static uint32_t last_changed = 0;
  bool reading = bottom_pin_->digital_read();
  uint32_t now = millis();

  // If value changed, whether noise or contact
  if (reading != last_state) {
    last_changed = now;
  }

  if ((now - last_changed) > this->debounce_) {
    state = reading;
  }

  last_state = reading;
  return state;
}

float TwoPinGarageSensor::get_setup_priority() const { return setup_priority::HARDWARE; }

void TwoPinGarageSensor::dump_config() {
  LOG_TEXT_SENSOR("", "TWO PIN GARAGE", this);
  LOG_PIN("  Top Pin: ", this->top_pin_);
  LOG_PIN("  Bottom Pin: ", this->bottom_pin_);
  ESP_LOGCONFIG(TAG, "  Debounce duration: %ums", this->debounce_);
}

}  // namespace two_pin_garage
}  // namespace esphome