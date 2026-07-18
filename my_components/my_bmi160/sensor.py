import esphome.codegen as cg
import esphome.components.bmi160.sensor as bmi160
import esphome.config_validation as cv
from esphome.components import i2c

DEPENDENCIES = bmi160.DEPENDENCIES
AUTO_LOAD = ["bmi160"]

bmi160_ns = cg.esphome_ns.namespace("my_bmi160")
BMI160Component = bmi160_ns.class_("MyBMI160Component", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = bmi160.CONFIG_SCHEMA.extend({cv.GenerateID(): cv.declare_id(BMI160Component)})


async def to_code(config):
    return await bmi160.to_code(config)
