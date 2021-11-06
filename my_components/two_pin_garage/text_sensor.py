import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import text_sensor
from esphome.const import CONF_ID, CONF_ICON, CONF_DEBOUNCE

CONF_TOP_PIN = "top_pin"
CONF_BOTTOM_PIN = "bottom_pin"

two_pin_garage_ns = cg.esphome_ns.namespace("two_pin_garage")
TwoPinGarageSensor = two_pin_garage_ns.class_("TwoPinGarageSensor", text_sensor.TextSensor, cg.Component)

CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(TwoPinGarageSensor),
    cv.Optional(CONF_ICON, default="mdi:garage"): cv.icon,
    cv.Required(CONF_TOP_PIN): pins.internal_gpio_input_pin_schema,
    cv.Required(CONF_BOTTOM_PIN): pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_DEBOUNCE, default="250ms"): cv.positive_time_period_milliseconds
}).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield text_sensor.register_text_sensor(var, config)

    top_pin = yield cg.gpio_pin_expression(config[CONF_TOP_PIN])
    cg.add(var.set_top_pin(top_pin))
    bottom_pin = yield cg.gpio_pin_expression(config[CONF_BOTTOM_PIN])
    cg.add(var.set_bottom_pin(bottom_pin))

    cg.add(var.set_debounce_duration(config[CONF_DEBOUNCE]))