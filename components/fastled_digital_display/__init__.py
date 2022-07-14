from string import digits

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, display, light
from esphome.components import fastled_base
import esphome.components.fastled_clockless.light as fastled_light
from esphome import pins
from esphome.const import (
    CONF_ID,
    CONF_CHIPSET,
    CONF_NUM_LEDS,
    CONF_PIN,
    CONF_RGB_ORDER,
    CONF_LAMBDA,
)

CONF_SEGMENTS = "segments"
CONF_SEGMENT_LEDS = "segment_leds"
CONF_LIGHT_STATE = "light_state"
CONF_LIGHT_OUTPUT = "light_output"


DEPENDENCIES = []

AUTO_LOAD = ["sensor", "light"]
MULTI_CONF = True

CONF_CONTROLLER_ID = "fastled_digital_display_id"

light_namespace = cg.esphome_ns.namespace("light")

sensor_namespace = cg.esphome_ns.namespace("fastled_digital_display")

Display = sensor_namespace.class_(
    "FastLEDDigitalDisplay", cg.Component, cg.PollingComponent
)
DisplayRef = Display.operator("ref")


def digits_only(value):
    value = cv.string(value)
    if not value:
        raise cv.Invalid("Value must not be empty")
    valid_chars = digits
    for char in value:
        if char not in valid_chars:
            raise cv.Invalid(
                f"IDs must only consist of numbers. The character '{char}' cannot be used"
            )
    return value


CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(Display),
        cv.Required(CONF_PIN): pins.internal_gpio_output_pin_number,
        cv.Required(CONF_SEGMENTS): digits_only,
        cv.Required(CONF_SEGMENT_LEDS): digits_only,
        cv.GenerateID(CONF_LIGHT_STATE): cv.use_id(light.LightState),
        cv.GenerateID(CONF_LIGHT_OUTPUT): cv.use_id(light.AddressableLight),
    }
).extend(cv.polling_component_schema("1s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_segments(config[CONF_SEGMENTS]))
    cg.add(var.set_segment_leds(config[CONF_SEGMENT_LEDS]))

    state = await cg.get_variable(config[CONF_LIGHT_STATE])
    output = await cg.get_variable(config[CONF_LIGHT_OUTPUT])
    cg.add(var.set_light_state(state))
    cg.add(var.set_light_output(output))
    await cg.register_component(var, config)
    await display.register_display(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
