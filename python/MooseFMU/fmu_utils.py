"""Helper utilities shared by FMU integration tooling and the Moose2FMU base class."""

from __future__ import annotations

from typing import Any, Dict, Optional

import logging

from fmpy import read_model_description

logger = logging.getLogger(__name__)


def configure_fmu_logging(
    *,
    debug: bool = False,
    logger_name: Optional[str] = None,
) -> logging.Logger:
    """Configure consistent logging for example FMU runner scripts.

    Parameters
    ----------
    debug
        When ``True`` the log level is elevated to ``DEBUG`` for both the root
        logger and the Moose2FMU namespace. ``False`` configures ``INFO`` level
        logging.
    logger_name
        Optional logger name to return. Defaults to the caller module's name when
        omitted.

    Returns
    -------
    logging.Logger
        The logger instance associated with ``logger_name`` (or the module
        logger for this helper when omitted).
    """

    level = logging.DEBUG if debug else logging.INFO

    # ``basicConfig`` only configures logging once; subsequent calls are
    # ignored when handlers are already present. This allows callers to
    # override the configuration in specialized scenarios while still sharing
    # the common defaults here.
    if not logging.getLogger().handlers:
        logging.basicConfig(
            level=level,
            format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        )

    root_logger = logging.getLogger()
    root_logger.setLevel(level)

    script_logger = logging.getLogger(logger_name or __name__)
    script_logger.setLevel(level)

    # Keep urllib3 connection pool chatter quiet unless explicitly requested.
    urllib3_logger = logging.getLogger("urllib3.connectionpool")
    urllib3_logger.propagate = False
    urllib3_logger.disabled = True

    moose_logger = logging.getLogger("Moose2FMU")
    moose_logger.setLevel(level)

    if debug:
        script_logger.debug("FMU debug logging is enabled")

    return script_logger

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
