import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, ble_client
from esphome.const import CONF_ID

DEPENDENCIES = ['esp32']

AUTO_LOAD = []
MULTI_CONF = True

CONF_CONTROLLER_ID = 'fastled_digital_display_id'

sensor_namespace = cg.esphome_ns.namespace('fastled_digital_display')

BLESensor = sensor_namespace.class_(
    'TextDisplay')

CONFIG_SCHEMA = cv.All(
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)