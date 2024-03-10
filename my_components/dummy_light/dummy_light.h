#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/light/light_output.h"

namespace esphome {
namespace dummy_light {

static const char *TAG = "dummy_light.light";

class DummyLightOutput : public light::LightOutput, public Component {
    public:
        void setup() override {};

        light::LightTraits get_traits() override {
            auto traits = light::LightTraits();
            traits.set_supported_color_modes({light::ColorMode::ON_OFF});
            return traits;
        };

        void write_state(light::LightState *state) override {}

        void dump_config() override {
            ESP_LOGCONFIG(TAG, "Empty custom light");
        };
};

} //namespace dummy_light
} //namespace esphome