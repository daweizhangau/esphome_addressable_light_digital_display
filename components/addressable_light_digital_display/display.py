import copy

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, light
from esphome.const import (
    CONF_ID,
    CONF_LAMBDA,
    CONF_ADDRESSABLE_LIGHT_ID,
    CONF_LIGHT_ID,
    CONF_UPDATE_INTERVAL,
)

CODEOWNERS = ["@daweizhangau"]

CONF_LED_MAP = "led_map"
CONF_REVERSE = "reverse"

addressable_light_ns = cg.esphome_ns.namespace("addressable_light_digital_display")
AddressableLightDisplay = addressable_light_ns.class_(
    "DigitalDisplay", cg.PollingComponent
)
AddressableLightDisplayRef = AddressableLightDisplay.operator("ref")


def led_map(value):
    value = cv.string(value)
    if not value:
        raise cv.Invalid("Value must not be empty")
    valid_chars = "XABCDEFG.: "
    for char in value:
        if char not in valid_chars:
            raise cv.Invalid(
                f"led_map must only consist of '{valid_chars}'. The character '{char}' cannot be used"
            )
    return value


CONFIG_SCHEMA = cv.All(
    display.BASIC_DISPLAY_SCHEMA.extend(light.RGB_LIGHT_SCHEMA).extend(
        {
            cv.GenerateID(): cv.declare_id(AddressableLightDisplay),
            cv.Required(CONF_ADDRESSABLE_LIGHT_ID): cv.use_id(
                light.AddressableLightState
            ),
            cv.Required(CONF_LED_MAP): led_map,
            cv.Optional(CONF_REVERSE, default=True): cv.boolean,
            cv.Optional(
                CONF_UPDATE_INTERVAL, default="16ms"
            ): cv.positive_time_period_milliseconds,
            cv.GenerateID(CONF_LIGHT_ID): cv.declare_id(light.AddressableLightState),
        }
    )
)


async def register_external_light(config, display_var):
    light_config = copy.deepcopy(config)
    light_config[CONF_ID] = light_config[CONF_LIGHT_ID]
    del light_config[CONF_UPDATE_INTERVAL]
    del light_config[CONF_LIGHT_ID]
    await light.register_light(display_var, light_config)


async def register_display(config, internal_light, display_var):
    cg.add(display_var.set_internal_light(internal_light))
    _led_map = config[CONF_LED_MAP]
    cg.add(display_var.set_led_map(_led_map))
    cg.add(display_var.set_max_characters(len(_led_map.split(" "))))
    cg.add(display_var.set_reverse(config[CONF_REVERSE]))
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(AddressableLightDisplayRef, "it")],
            return_type=cg.void,
        )
        cg.add(display_var.set_writer(lambda_))
    await display.register_display(display_var, config)


async def to_code(config):
    internal_light = await cg.get_variable(config[CONF_ADDRESSABLE_LIGHT_ID])
    digital_display = cg.new_Pvariable(config[CONF_ID])

    await register_display(config, internal_light, digital_display)
    await register_external_light(config, digital_display)
