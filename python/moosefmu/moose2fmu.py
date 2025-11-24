# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

try:
    from pythonfmu import Fmi2Slave
    from pythonfmu.enums import Fmi2Causality, Fmi2Variability
    from pythonfmu.variables import Integer, Real, String
except ModuleNotFoundError as exc:
    raise ModuleNotFoundError(
        "pythonfmu is required to build and run FMUs. Install it in the same Python "
        "environment used to execute MooseFMU tests (e.g., `pip install pythonfmu`) "
        "or add that environment to PYTHONPATH."
    ) from exc

import logging
import numbers
import re
import shlex
import time
from typing import Dict, Iterable, Optional, Set, Tuple, Union

from MooseControl import MooseControl


class Moose2FMU(Fmi2Slave):
    """
    Base FMU slave for MOOSE simulations. Handles registration of FMU variables,
    control setup, and stepping logic stub to be implemented by subclasses.
    """

    def __init__(
        self,
        *args,
        flag: str = "",
        moose_command: str = "",
        server_name: str = "web_server",
        max_retries: int = 5,
        dt_tolerance: Real = 1e-3,
        **kwargs,
    ):
        super().__init__(*args, **kwargs, logging_add_standard_categories=True)

        base_logger = logging.getLogger(self.__class__.__module__)
        self.logger = base_logger.getChild(self.__class__.__name__)
        self.logger.info("Moose2FMU initialized successfully.")

        # Configuration parameters
        self.flag: str = flag
        self.moose_command: str = moose_command
        self.server_name: str = server_name
        self.max_retries: int = max_retries
        self.dt_tolerance: Real = dt_tolerance
        self.moose_time: Real = 0.0
        self.start_time: Real = (0.0,)
        self.stop_time: Real = (0.0,)
        self.tolerance: Real = (1.0e-3,)

        # Default synchronization and data retrieval flags
        self._default_sync_flags: Set[str] = {"INITIAL", "MULTIAPP_FIXED_POINT_BEGIN"}
        self._default_data_flags: Set[str] = {"MULTIAPP_FIXED_POINT_END"}

        # Register a default set of parameters
        self.register_variable(
            String(
                "flag",
                causality=Fmi2Causality.parameter,
                variability=Fmi2Variability.tunable,
            )
        )
        self.register_variable(
            String(
                "moose_command",
                causality=Fmi2Causality.parameter,
                variability=Fmi2Variability.tunable,
            )
        )
        self.register_variable(
            String(
                "server_name",
                causality=Fmi2Causality.parameter,
                variability=Fmi2Variability.tunable,
            )
        )
        self.register_variable(
            Integer(
                "max_retries",
                causality=Fmi2Causality.parameter,
                variability=Fmi2Variability.tunable,
            )
        )
        self.register_variable(
            Real(
                "dt_tolerance",
                causality=Fmi2Causality.parameter,
                variability=Fmi2Variability.tunable,
            )
        )

        # Register outputs
        self.register_variable(
            Real(
                "moose_time",
                causality=Fmi2Causality.output,
                variability=Fmi2Variability.continuous,
            )
        )

        # Track previously applied controllable values so repeated requests can be skipped
        self._controllable_real_cache: Dict[str, float] = {}
        self._controllable_vector_cache: Dict[
            Tuple[str, str], Tuple[Union[float, int, str], ...]
        ] = {}

    def exit_initialization_mode(self) -> bool:

        # Setup MooseControl
        cmd = shlex.split(self.moose_command)
        self.control = MooseControl(
            moose_command=cmd, moose_control_name=self.server_name
        )
        self.control.initialize()

        return True

    def setup_experiment(
        self, start_time: float, stop_time: Optional[float], tolerance: Optional[float]
    ) -> bool:
        """
        Save an local copy of start_time, stop_time and tolerance.
        """
        self.start_time = start_time
        self.stop_time = stop_time
        self.tolerance = tolerance
        return True

    def do_step(
        self,
        current_time: float,
        step_size: float,
        no_set_fmu_state_prior: bool = False,
    ) -> bool:
        """
        FMU stepping logic must be implemented by subclasses.
        """
        raise NotImplementedError("Subclasses must implement do_step()")

    def sync_with_moose(
        self,
        current_time: float,
        step_size: float,
        allowed_flags: Optional[Union[str, Iterable[str]]] = None,
    ) -> Tuple[Optional[float], Optional[str]]:
        """
        Synchronize with the running MOOSE simulation.

        Parameters
        ----------
        current_time
            Target FMU time to synchronize with.
        allowed_flags
            Subset of flags that require additional handling. Defaults to
            synchronizing on the ``INITIAL`` and ``MULTIAPP_FIXED_POINT_BEGIN`` signals
            when omitted.

        Returns
        -------
        tuple
            A pair containing the synchronized MOOSE time and the flag that
            triggered the synchronization (if any).

        """
        parsed_allowed_flags = self._combine_flags(
            self._default_sync_flags,
            self._default_data_flags,
            allowed_flags,
            self._user_defined_flags(),
        )

        if parsed_allowed_flags:
            self.logger.info(
                "Allowed flags for handling: %s", sorted(parsed_allowed_flags)
            )

        signal: Optional[str] = None

        while self.control.isProcessRunning():
            flag = self.get_flag_with_retries(parsed_allowed_flags, self.max_retries)
            if not flag:
                self.logger.error(f"Failed to fast-forward to {current_time}")
                self.control.finalize()
                return None, None

            moose_time = self.control.getTime()

            if flag in parsed_allowed_flags:
                signal = flag

                # set next time in MOOSE, ensure MOOSE has data avaiable for all FMU times (Optional)
                # To-do: need MooseControl create a time object and a postprocessor backend,
                # then we don't need users adding extra blocks in their input files
                if signal in ["INITIAL", "MULTIAPP_FIXED_POINT_BEGIN"]:
                    moose_dt = self.control.getDT()

                    self.logger.debug(
                        "moose_time=%.6f -> current_time=%.6f -> moose_dt=%.6f -> step_size=%.6f",
                        moose_time,
                        current_time,
                        moose_dt,
                        step_size,
                    )

                    if moose_dt > step_size:
                        logging.info(
                            "moose_dt (%s) must be <= step_size (%s).",
                            moose_dt,
                            step_size,
                        )
                        return None, None

            if abs(moose_time - current_time) < self.dt_tolerance:
                self.logger.debug("Captured synchronization flag '%s'", flag)
                self.logger.debug(
                    f"The current time is {current_time}, the moose time is {moose_time}"
                )
                self.logger.info("Successfully sync MOOSE time with FMU step")

                self.moose_time = moose_time

                return moose_time, signal

            self._skip_flag(flag)

    def ensure_control_listening(self) -> bool:
        """Verify the MooseControl server is listening before proceeding."""
        if self.control.isListening():
            return True

        self.logger.error("MOOSE is not listening")
        self.control.finalize()
        return False

    def set_controllable_real(
        self,
        path: str,
        value: float,
        *,
        force: bool = False,
        flag: Optional[Union[str, Iterable[str]]] = None,
    ) -> bool:
        """
        Set a controllable ``Real`` parameter, skipping redundant updates.

        Parameters
        ----------
        path
            The controllable parameter path recognized by WebServerControl.
        value
            The value that should be applied.
        force
            When ``True`` the value will be pushed even if it matches the last
            applied value.
        flag
            Optional flag (or collection of flags) that must be received from
            ``MooseControl`` before sending the update. When omitted the
            user-defined flag configured through ``self.flag`` is used. If no
            flag is available the value is sent immediately.

        Returns
        -------
        bool
            ``True`` if a new value was sent to MOOSE, ``False`` when the
            request was skipped because it matches the previously applied value.

        """
        cached = self._controllable_real_cache.get(path)
        if not force and cached == value:
            self.logger.debug(
                "Skipping controllable real update for '%s'; value %s already applied",
                path,
                value,
            )
            return False

        wait_flags = self._combine_flags(flag, self._user_defined_flags())
        if wait_flags:
            awaited = self._wait_for_flags(
                wait_flags,
                context=f"Failed to receive flag before updating controllable '{path}'",
            )
            if awaited is None:
                return False

        self.control.setControllableReal(path, value)
        self.control.setContinue()
        self._controllable_real_cache[path] = value
        self.logger.info("Set controllable real '%s' to %s", path, value)
        return True

    def set_controllable_vector(
        self,
        path: str,
        value: Iterable[Union[float, int, str]],
        *,
        value_type: Optional[str] = None,
        force: bool = False,
        flag: Optional[Union[str, Iterable[str]]] = None,
    ) -> bool:
        """
        Set a controllable vector parameter using the appropriate MooseControl API.

        Parameters
        ----------
        path
            The controllable parameter path recognized by WebServerControl.
        value
            Iterable containing the values that should be applied.
        value_type
            Optional hint describing the type of the vector elements. Accepted
            values are ``"real"``, ``"int"``/``"integer```, and ``"string"``.
            When omitted the type is inferred from the provided values.
        force
            When ``True`` the value will be pushed even if it matches the last
            applied value.
        flag
            Optional flag (or collection of flags) that must be received from
            ``MooseControl`` before sending the update. When omitted the
            user-defined flag configured through ``self.flag`` is used. If no
            flag is available the value is sent immediately.


        Returns
        -------
        bool
            ``True`` if a new value was sent to MOOSE, ``False`` when the
            request was skipped because it matches the previously applied value.

        Raises
        ------
        TypeError
            When ``value`` is not an iterable of scalars, or when a type cannot
            be inferred and ``value_type`` is omitted.
        ValueError
            When attempting to set an empty vector without specifying a
            ``value_type``.

        """
        if isinstance(value, (str, bytes)):
            raise TypeError("'value' must be an iterable of scalar entries")

        if hasattr(value, "tolist"):
            raw_values = value.tolist()
        else:
            raw_values = list(value)

        if not raw_values and value_type is None:
            raise ValueError(
                "value_type must be provided when assigning an empty vector"
            )

        def _is_integral(entry: object) -> bool:
            return isinstance(entry, numbers.Integral) and not isinstance(entry, bool)

        def _is_numeric(entry: object) -> bool:
            return isinstance(entry, numbers.Real) and not isinstance(entry, bool)

        normalized_type: Optional[str] = None
        if value_type:
            normalized = value_type.strip().lower()
            if normalized in {"real", "reals", "float", "double"}:
                normalized_type = "real"
            elif normalized in {"int", "integer", "integers"}:
                normalized_type = "int"
            elif normalized in {"string", "strings", "str"}:
                normalized_type = "string"
            else:
                raise TypeError(f"Unsupported value_type '{value_type}'")
        elif raw_values:
            if all(isinstance(entry, str) for entry in raw_values):
                normalized_type = "string"
            elif all(_is_integral(entry) for entry in raw_values):
                normalized_type = "int"
            elif all(_is_numeric(entry) for entry in raw_values):
                normalized_type = "real"
            else:
                raise TypeError(
                    "Unable to infer vector type from heterogeneous values; provide value_type"
                )

        assert normalized_type is not None

        if normalized_type == "real":
            converted = tuple(float(entry) for entry in raw_values)
            setter = self.control.setControllableVectorReal
        elif normalized_type == "int":
            converted = tuple(int(entry) for entry in raw_values)
            setter = self.control.setControllableVectorInt
        else:
            converted = tuple(str(entry) for entry in raw_values)
            setter = self.control.setControllableVectorString

        cache_key = (path, normalized_type)
        cached = self._controllable_vector_cache.get(cache_key)
        if not force and cached == converted:
            self.logger.debug(
                "Skipping controllable vector update for '%s'; value %s already applied",
                path,
                list(converted),
            )
            return False

        wait_flags = self._combine_flags(flag, self._user_defined_flags())
        if wait_flags:
            awaited = self._wait_for_flags(
                wait_flags,
                context=f"Failed to receive flag before updating controllable '{path}'",
            )
            if awaited is None:
                return False

        setter(path, list(converted))
        self.control.setContinue()
        self._controllable_vector_cache[cache_key] = converted
        self.logger.info(
            "Set controllable vector '%s' (%s) to %s",
            path,
            normalized_type,
            list(converted),
        )
        return True

    def get_flag_with_retries(
        self,
        allowed_flags: Optional[Union[str, Iterable[str]]],
        max_retries: int,
        wait_seconds: float = 0.5,
    ) -> Optional[str]:
        """
        Poll ``getWaitingFlag`` and retry before giving up.

        Parameters
        ----------
        allowed_flags
            Optional collection (or delimited string) of flags expected from
            ``MooseControl``. Used for logging and normalizing the responses.
        max_retries
            Number of times to attempt retrieving the flag.
        wait_seconds
            Delay between retries in seconds.

        """
        normalized_allowed: Optional[Set[str]] = None
        if allowed_flags:
            normalized_allowed = self._parse_flags(allowed_flags)

        retries = 0
        result: Optional[str] = None
        while retries < max_retries:
            try:
                result = self.control.getWaitingFlag()
                if result:
                    normalized = result.strip().upper()
                    if normalized_allowed and normalized not in normalized_allowed:
                        self.logger.debug(
                            "Received flag '%s' not present in allowed set %s",
                            result,
                            sorted(normalized_allowed),
                        )
                        try:
                            self._skip_flag(result)

                        except Exception as exc:
                            self.logger.warning(
                                "Failed to acknowledge unexpected flag '%s': %s",
                                result,
                                exc,
                            )
                        continue
                    self.logger.info(
                        f"Successfully got flag '{result}' after {retries} retries."
                    )
                    return result
            except Exception as exc:
                self.logger.warning(
                    f"Attempt {retries + 1}/{max_retries} failed: {exc}"
                )

            retries += 1
            self.logger.info(f"Waiting {wait_seconds} seconds before retrying...")
            time.sleep(wait_seconds)

        allowed_desc = (
            f"one of {sorted(normalized_allowed)}" if normalized_allowed else "any flag"
        )
        self.logger.error(
            "Failed to get %s after %s retries.", allowed_desc, max_retries
        )
        return None

    def get_postprocessor_value(
        self,
        postprocessor_name: str,
        current_time: float,
        flag: Optional[Union[str, Iterable[str]]] = None,
    ) -> Optional[float]:
        """
        Wait for a synchronization flag and fetch a postprocessor value.

        Returns
        -------
        Optional[float]
            ``None`` when the flag retrieval fails; otherwise the fetched postprocessor
            value.

        """
        wait_flags = self._combine_flags(
            self._default_data_flags,
            flag,
            self._user_defined_flags(),
        )
        flag_value = self._wait_for_flags(
            wait_flags,
            context=f"Failed to fast-forward to {current_time}",
        )

        if flag_value is None:
            return None

        postprocessor_value = self.control.getPostprocessor(postprocessor_name)
        self.logger.debug(
            f"Retrieved Postprocessor value {postprocessor_value} from MOOSE at flag {flag_value}"
        )

        return postprocessor_value

    def get_reporter_value(
        self,
        reporter_name: str,
        current_time: float,
        flag: Optional[Union[str, Iterable[str]]] = None,
    ) -> Optional[float]:
        """
        Wait for a synchronization flag and fetch a reporter value.

        Returns
        -------
        Optional[float]
            ``None`` when the flag retrieval fails; otherwise the fetched reporter
            value.

        """
        wait_flags = self._combine_flags(
            self._default_data_flags,
            flag,
            self._user_defined_flags(),
        )
        flag_value = self._wait_for_flags(
            wait_flags,
            context=f"Failed to fast-forward to {current_time}",
        )

        if flag_value is None:
            return None

        reporter_value = self.control.getReporterValue(reporter_name)
        self.logger.debug(
            f"Retrieved Reporter value {reporter_value} from MOOSE at flag {flag_value}"
        )

        return reporter_value

    def _parse_flags(self, flags: Union[str, Iterable[str]]) -> Set[str]:
        """
        Normalize flags to an uppercase set.
        Accepts a space/comma/semicolon/pipe-separated string or an iterable.
        """
        if isinstance(flags, str):
            tokens = re.split(r"[,\s;|]+", flags.strip())
        else:
            tokens = list(flags)
        return {t.strip().upper() for t in tokens if t and t.strip()}

    def _skip_flag(self, flag: str):
        """Acknowledge a flag and allow the MOOSE execution to continue."""
        self.control.wait(flag)
        self.control.setContinue()

    def _combine_flags(
        self,
        *flag_groups: Optional[Union[str, Iterable[str]]],
    ) -> Set[str]:
        combined: Set[str] = set()
        for group in flag_groups:
            if not group:
                continue
            combined |= self._parse_flags(group)
        return combined

    def _user_defined_flags(self) -> Set[str]:
        if not self.flag:
            return set()
        return self._parse_flags(self.flag)

    def _wait_for_flags(
        self,
        flags: Set[str],
        *,
        context: Optional[str] = None,
    ) -> Optional[str]:
        if not flags:
            return ""

        self.logger.debug("Waiting for one of the flags: %s", sorted(flags))
        flag_value = self.get_flag_with_retries(flags, self.max_retries)

        if not flag_value:
            if context:
                self.logger.error(context)
            else:
                self.logger.error(
                    "Failed to receive any of the expected flags: %s",
                    sorted(flags),
                )
            self.control.finalize()
            return None

        self.control.wait(flag_value)
        normalized_flag = flag_value.strip().upper()
        self.logger.debug("Received flag '%s'", normalized_flag or flag_value)
        return normalized_flag or flag_value

    def _schedule_next_time(self, next_time: float) -> bool:
        """
        Request MOOSE to insert an additional time point if needed.

        Parameters
        ----------
        next_time
            The time that MOOSE should be forced to hit before advancing further.

        Returns
        -------
        bool
            ``True`` when a new request was sent to MOOSE, ``False`` if it was
            skipped because the same value was already applied.

        """
        path = "Times/external_input/next_time"
        cached = self._controllable_real_cache.get(path)
        if cached == next_time:
            self.logger.debug(
                "Skipping next-time scheduling; %.6f already requested", next_time
            )
            return False

        self.control.setControllableReal(path, next_time)
        self._controllable_real_cache[path] = next_time
        return True
