from pythonfmu import Fmi2Slave
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Integer, Real, String
from pythonfmu.default_experiment import DefaultExperiment
from MooseControl import MooseControl
from typing import List, Dict, Optional
import logging
import time
from . import fmu_utils
from typing import Iterable, Union, Set, Tuple
import re
import shutil
import mooseutils
import numbers

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s - %(message)s",
    handlers=[
        logging.FileHandler("moose_simulation.log"),
        logging.StreamHandler()
    ]
)

class Moose2FMU(Fmi2Slave):
    """
    Base FMU slave for MOOSE simulations. Handles registration of FMU variables,
    control setup, and stepping logic stub to be implemented by subclasses.
    """
    def __init__(
        self,
        *args,
        flag: str = "",
        moose_mpi: str = "",
        mpi_num: str = 1,
        moose_executable: str = "",
        moose_inputfile: str = "",
        server_name: str = "web_server",
        max_retries: int = 5,
        dt_tolerance: Real = 1e-3,
        **kwargs,
    ):
        super().__init__(*args, **kwargs, logging_add_standard_categories=True)

        self.logger = logging.getLogger(self.__class__.__name__)
        self.logger.info("Moose2FMU initialized successfully.")

        # Configuration parameters
        self.flag: str = flag
        self.moose_mpi: str = moose_mpi
        self.mpi_num: str = mpi_num
        self.moose_inputfile: str = moose_inputfile
        self.server_name: str = server_name
        self.max_retries: int = max_retries
        self.dt_tolerance: Real = dt_tolerance
        self.moose_time: Real = 0.0
        # self.begin_time: Real = 0.0
        # self.end_time: Real = float("inf")

        # Find moose executable
        exec = shutil.which("moose-opt") or mooseutils.find_moose_executable_recursive()
        if not exec and moose_executable:
            raise AssertionError("Cannot find a valid MOOSE executable.")

        self.moose_executable: str = exec

        # Register tunable parameters
        self.register_variable(String("flag", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(String("moose_mpi", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(String("mpi_num", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(String("moose_executable", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(String("moose_inputfile", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(String("server_name", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(Integer("max_retries", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(Real("dt_tolerance", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))

        # Register outputs
        self.register_variable(Real("moose_time", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))


        # Setup MooseControl
        self.cmd = self._build_moose_command()

        # Track previously applied controllable values so repeated requests can be skipped
        self._controllable_real_cache: Dict[str, float] = {}
        self._controllable_vector_cache: Dict[Tuple[str, str], Tuple[Union[float, int, str], ...]] = {}

    def exit_initialization_mode(self) -> bool:

        self.cmd = self._build_moose_command()
        self.control = MooseControl(moose_command=self.cmd, moose_control_name=self.server_name)
        self.control.initialize()

        return True

    # def setup_experiment(self, start_time: float, stop_time: Optional[float], tolerance: Optional[float]):
    #     self.begin_time = start_time
    #     if stop_time:
    #         self.end_time = stop_time
    #     else:
    #         self.end_time = float("inf")
    #     pass

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
        """Synchronize with the running MOOSE simulation.

        Parameters
        ----------
        current_time
            Target FMU time to synchronize with.
        allowed_flags
            Subset of flags that require additional handling. Defaults to
            synchronizing on the ``INITIAL`` and ``TIMESTEP_BEGIN`` signals
            when omitted.
        Returns
        -------
        tuple
            A pair containing the synchronized MOOSE time and the flag that
            triggered the synchronization (if any).
        """

        parsed_allowed_flags = {"INITIAL", "TIMESTEP_BEGIN"}
        if allowed_flags:
            parsed_allowed_flags |= self._parse_flags(allowed_flags)

        if parsed_allowed_flags:
            self.logger.info(
                "Allowed flags for handling: %s", sorted(parsed_allowed_flags)
            )

        signal: Optional[str] = None

        while True:
            flag = self.get_flag_with_retries(parsed_allowed_flags, self.max_retries)
            if not flag:
                self.logger.error(f"Failed to fast-forward to {current_time}")
                self.control.finalize()
                return None, None

            moose_time = self.control.getTime()

            if flag in parsed_allowed_flags:
                signal = flag
                self.logger.debug("Captured synchronization flag '%s'", flag)

            if abs(moose_time - current_time) < self.dt_tolerance:
                self.logger.debug(f"The current time is {current_time}, the moose time is {moose_time}")
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

    def set_controllable_real(self, path: str, value: float, *, force: bool = False) -> bool:
        """Set a controllable ``Real`` parameter, skipping redundant updates.

        Parameters
        ----------
        path
            The controllable parameter path recognized by WebServerControl.
        value
            The value that should be applied.
        force
            When ``True`` the value will be pushed even if it matches the last
            applied value.

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

        self.control.setControllableReal(path, value)
        self.control.setContinue()
        self._controllable_real_cache[path] = value
        self.logger.info(
            "Set controllable real '%s' to %s", path, value
        )
        return True

    def set_controllable_vector(
        self,
        path: str,
        value: Iterable[Union[float, int, str]],
        *,
        value_type: Optional[str] = None,
        force: bool = False,
    ) -> bool:
        """Set a controllable vector parameter using the appropriate MooseControl API.

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
            raise ValueError("value_type must be provided when assigning an empty vector")

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
        self, allowed_flags: Optional[Set[str]], max_retries: int, wait_seconds: float = 0.5
    ) -> Optional[str]:
        """Poll ``getWaitingFlag`` and retry before giving up.

        Parameters
        ----------
        allowed_flags
            Optional set of flags expected from ``MooseControl``. Used for logging and
            normalizing the responses.
        max_retries
            Number of times to attempt retrieving the flag.
        wait_seconds
            Delay between retries in seconds.
        """
        retries = 0
        result: Optional[str] = None
        while retries < max_retries:
            try:
                result = self.control.getWaitingFlag()
                if result:
                    normalized = result.strip().upper()
                    if allowed_flags and normalized not in allowed_flags:
                        self.logger.debug(
                            "Received flag '%s' not present in allowed set %s",
                            result,
                            sorted(allowed_flags),
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
            self.logger.info(
                f"Waiting {wait_seconds} seconds before retrying..."
            )
            time.sleep(wait_seconds)

        allowed_desc = (
            f"one of {sorted(allowed_flags)}" if allowed_flags else "any flag")
        self.logger.error(
             "Failed to get %s after %s retries.", allowed_desc, max_retries)
        return None

    def get_postprocessor_value(
        self, flag: str, postprocessor_name: str, current_time: float
    ) -> Optional[float]:
        """Wait for ``flag`` and fetch ``postprocessor_name`` from the control server.

        Returns
        -------
        Optional[float]
            ``None`` when the flag retrieval fails; otherwise the fetched postprocessor
            value.
        """

        flag_value = self.get_flag_with_retries(flag, self.max_retries)

        if not flag_value:
            self.logger.error(f"Failed to fast-forward to {current_time}")
            self.control.finalize()
            return None

        self.control.wait(flag_value)
        postprocessor_value = self.control.getPostprocessor(postprocessor_name)
        self.control.setContinue()
        self.logger.info(
            f"Retrieved Postprocessor value {postprocessor_value} from MOOSE at flag {flag_value}"
        )

        return postprocessor_value


    def get_reporter_value(
        self, flag: str, reporter_name: str, current_time: float
    ) -> Optional[float]:
        """Wait for ``flag`` and fetch ``reporter_name`` from the control server.

        Returns
        -------
        Optional[float]
            ``None`` when the flag retrieval fails; otherwise the fetched reporter
            value.
        """

        flag_value = self.get_flag_with_retries(flag, self.max_retries)

        if not flag_value:
            self.logger.error(f"Failed to fast-forward to {current_time}")
            self.control.finalize()
            return None

        self.control.wait(flag_value)
        reporter_value = self.control.getReporterValue(reporter_name)
        self.control.setContinue()
        self.logger.info(
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

    def _build_moose_command(self) -> List[str]:
        if self.moose_mpi:
            return [
                self.moose_mpi,
                "-n",
                str(self.mpi_num),
                self.moose_executable,
                "-i",
                self.moose_inputfile,
            ]
        return [self.moose_executable, "-i", self.moose_inputfile]


