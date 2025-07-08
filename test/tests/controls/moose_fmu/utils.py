import numpy as np
from fmpy import read_model_description
import logging

logger = logging.getLogger(__name__)


def fmu_info(fmu_model: str, filename: str):
    logger.info(f"Load fmu model decription: {filename}")
    fmu_description = read_model_description(fmu_model)

    logger.info("Fmu model info:")
    for v in fmu_description.modelVariables:
        logger.info(f"{v.name:10s}  causality={v.causality:10s}  variability={v.variability:12s}  "
            f"type={v.type}  start={v.start} valueReference={v.valueReference}")

    return fmu_description

def get_real(fmu, vr_map, name):
    vr = vr_map[name]
    return fmu.getReal([vr])[0]

def get_string(fmu, vr_map, name):
    vr = vr_map[name]
    return fmu.getString([vr])[0]

def get_bool(fmu, vr_map, name):
    vr = vr_map[name]
    return fmu.getBoolean([vr])[0]

def set_string(fmu, vr_map, name, value):
    vr = vr_map[name]
    return fmu.setString([vr], [value])

def set_real(fmu, vr_map, name, value):
    vr = vr_map[name]
    return fmu.setReal([vr], [value])

def set_bool(fmu, vr_map, name, value):
    vr = vr_map[name]
    return fmu.setBoolean([vr], [value])
