"""Helper utilities shared by FMU integration tooling and the Moose2FMU base class."""

from __future__ import annotations

from typing import Any, Dict

import logging

from fmpy import read_model_description

logger = logging.getLogger(__name__)


def fmu_info(fmu_model: str, filename: str):
    """Read and log the FMI model description for the given FMU archive."""

    logger.info("Load FMU model description: %s", filename)
    fmu_description = read_model_description(fmu_model)

    logger.info("FMU model info:")
    for variable in fmu_description.modelVariables:
        logger.info(
            "%s  causality=%s  variability=%s  type=%s  start=%s valueReference=%s",
            f"{variable.name:10s}",
            f"{variable.causality:10s}",
            f"{variable.variability:12s}",
            variable.type,
            variable.start,
            variable.valueReference,
        )

    return fmu_description


def get_real(fmu: Any, vr_map: Dict[str, int], name: str) -> float:
    """Retrieve a scalar real variable by name from an instantiated FMU."""

    value_reference = vr_map[name]
    return fmu.getReal([value_reference])[0]


def get_string(fmu: Any, vr_map: Dict[str, int], name: str) -> str:
    """Retrieve a scalar string variable by name from an instantiated FMU."""

    value_reference = vr_map[name]
    return fmu.getString([value_reference])[0]


def get_bool(fmu: Any, vr_map: Dict[str, int], name: str) -> bool:
    """Retrieve a scalar boolean variable by name from an instantiated FMU."""

    value_reference = vr_map[name]
    return fmu.getBoolean([value_reference])[0]


def set_string(fmu: Any, vr_map: Dict[str, int], name: str, value: str) -> None:
    """Assign a scalar string variable by name on an instantiated FMU."""

    value_reference = vr_map[name]
    fmu.setString([value_reference], [value])


def set_real(fmu: Any, vr_map: Dict[str, int], name: str, value: float) -> None:
    """Assign a scalar real variable by name on an instantiated FMU."""

    value_reference = vr_map[name]
    fmu.setReal([value_reference], [value])


def set_bool(fmu: Any, vr_map: Dict[str, int], name: str, value: bool) -> None:
    """Assign a scalar boolean variable by name on an instantiated FMU."""

    value_reference = vr_map[name]
    fmu.setBoolean([value_reference], [value])
