# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Helper utilities shared by FMU integration tooling and the Moose2FMU base class."""

from __future__ import annotations

import contextlib
import logging
from types import SimpleNamespace
from typing import Any, Dict, Optional

try:
    from pyfmi import load_fmu
except ModuleNotFoundError:
    load_fmu = None

logger = logging.getLogger(__name__)


def as_scalar(value: Any) -> Any:
    """Normalize scalar-like FMU values (arrays/lists) into a Python scalar."""
    if hasattr(value, "ndim") and hasattr(value, "size"):
        if value.ndim == 0:
            with contextlib.suppress(Exception):
                return value.item()

        if value.size == 1:
            with contextlib.suppress(Exception):
                return value.reshape(-1)[0].item()
            with contextlib.suppress(Exception):
                return value.reshape(-1)[0]
            with contextlib.suppress(Exception):
                return value[0]

        shape = getattr(value, "shape", "?")
        raise ValueError(f"Expected scalar FMU value, got array-like shape={shape}")

    if isinstance(value, (list, tuple)):
        if len(value) == 1:
            return as_scalar(value[0])
        raise ValueError(f"Expected scalar FMU value, got sequence length={len(value)}")

    return value


def get_scalar(fmu: Any, name: str) -> Any:
    """Retrieve a scalar FMU variable by name using ``fmu.get``."""
    if not hasattr(fmu, "get"):
        raise AttributeError("FMU object does not expose get()")
    return as_scalar(fmu.get(name))


def get_float(fmu: Any, name: str) -> float:
    """Retrieve a scalar FMU variable by name and convert it to ``float``."""
    return float(get_scalar(fmu, name))


def configure_fmu_logging(
    *,
    debug: bool = False,
    logger_name: Optional[str] = None,
) -> logging.Logger:
    """
    Configure consistent logging for example FMU runner scripts.

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
    """
    Read and log FMI model variable metadata for the given FMU archive.

    The reported ``start`` and ``valueReference`` fields are read from the FMU's
    ``modelDescription.xml``; they are not supplied to this function. ``start`` is
    the optional declared initial value of a variable, and ``valueReference`` is
    the integer handle the FMI API uses to address a variable by id instead of by
    name. Both are fixed when the FMU is built -- ``valueReference`` is
    auto-assigned by ``pythonfmu`` as each variable is registered -- so this
    function only introspects them.
    """
    if load_fmu is None:
        raise ModuleNotFoundError(
            "fmu_info() requires optional dependency 'pyfmi'. "
            "Install it (e.g. `pip install pyfmi`) to read FMU model metadata."
        )

    logger.info("Load FMU model description: %s", filename)
    model = load_fmu(fmu_model)

    try:
        variables = []
        for name, variable in model.get_model_variables().items():
            value_reference = getattr(
                variable, "value_reference", getattr(variable, "valueReference", None)
            )
            start_value = getattr(variable, "start", None)
            with contextlib.suppress(Exception):
                start_value = model.get_variable_start(name)

            variables.append(
                SimpleNamespace(
                    name=str(name),
                    causality=str(getattr(variable, "causality", "")),
                    variability=str(getattr(variable, "variability", "")),
                    type=str(getattr(variable, "type", type(variable).__name__)),
                    start=start_value,
                    valueReference=value_reference,
                )
            )
    finally:
        with contextlib.suppress(Exception):
            model.terminate()

    fmu_description = SimpleNamespace(modelVariables=variables)

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


def get_real(fmu: Any, vr_map: Dict[str, Any], name: str) -> float:
    """Retrieve a scalar real variable by name from an instantiated FMU."""
    if hasattr(fmu, "getReal"):
        value_reference = vr_map[name]
        return fmu.getReal([value_reference])[0]
    if hasattr(fmu, "get"):
        return get_float(fmu, name)
    raise AttributeError("FMU object does not expose getReal() or get()")


def get_string(fmu: Any, vr_map: Dict[str, Any], name: str) -> str:
    """Retrieve a scalar string variable by name from an instantiated FMU."""
    if hasattr(fmu, "getString"):
        value_reference = vr_map[name]
        return fmu.getString([value_reference])[0]
    if hasattr(fmu, "get"):
        return str(get_scalar(fmu, name))
    raise AttributeError("FMU object does not expose getString() or get()")


def get_bool(fmu: Any, vr_map: Dict[str, Any], name: str) -> bool:
    """Retrieve a scalar boolean variable by name from an instantiated FMU."""
    if hasattr(fmu, "getBoolean"):
        value_reference = vr_map[name]
        return fmu.getBoolean([value_reference])[0]
    if hasattr(fmu, "get"):
        return bool(get_scalar(fmu, name))
    raise AttributeError("FMU object does not expose getBoolean() or get()")


def set_string(fmu: Any, vr_map: Dict[str, Any], name: str, value: str) -> None:
    """Assign a scalar string variable by name on an instantiated FMU."""
    if hasattr(fmu, "setString"):
        value_reference = vr_map[name]
        fmu.setString([value_reference], [value])
        return
    if hasattr(fmu, "set"):
        fmu.set(name, value)
        return
    raise AttributeError("FMU object does not expose setString() or set()")


def set_real(fmu: Any, vr_map: Dict[str, Any], name: str, value: float) -> None:
    """Assign a scalar real variable by name on an instantiated FMU."""
    if hasattr(fmu, "setReal"):
        value_reference = vr_map[name]
        fmu.setReal([value_reference], [value])
        return
    if hasattr(fmu, "set"):
        fmu.set(name, value)
        return
    raise AttributeError("FMU object does not expose setReal() or set()")


def set_bool(fmu: Any, vr_map: Dict[str, Any], name: str, value: bool) -> None:
    """Assign a scalar boolean variable by name on an instantiated FMU."""
    if hasattr(fmu, "setBoolean"):
        value_reference = vr_map[name]
        fmu.setBoolean([value_reference], [value])
        return
    if hasattr(fmu, "set"):
        fmu.set(name, value)
        return
    raise AttributeError("FMU object does not expose setBoolean() or set()")
