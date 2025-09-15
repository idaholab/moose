from pythonfmu import Fmi2Slave
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Integer, Real, String
from pythonfmu.default_experiment import DefaultExperiment
from MooseControl import MooseControl
from typing import Optional
import logging
import time
from . import fmu_utils


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
        moose_executable: str = "../../../moose_test-opt",
        moose_inputfile: str = "fmu_diffusion.i",
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
        self.moose_executable: str = moose_executable
        self.moose_inputfile: str = moose_inputfile
        self.server_name: str = server_name
        self.max_retries: int = max_retries
        self.dt_tolerance: Real = dt_tolerance

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
        self.register_variable(Real("time", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))

        # Setup MooseControl
        if self.moose_mpi:
            self.cmd = [self.moose_mpi, "-n", self.mpi_num, self.moose_executable, "-i", self.moose_inputfile]
        else:
            self.cmd = [self.moose_executable, "-i", self.moose_inputfile]


    def exit_initialization_mode(self) -> bool:

        self.control = MooseControl(moose_command=self.cmd, moose_control_name=self.server_name)
        self.control.initialize()

        return True

    def setup_experiment(
        self,
        start_time: float,
        stop_time: Optional[float],
        tolerance: Optional[float],
    ) -> bool:
        self.sim_start_time = start_time
        self.sim_stop_time = stop_time
        self.sim_tolerance = tolerance
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

    def _get_flag_with_retries(
        self, flag: str, max_retries: int, wait_seconds: float = 0.5
    ) -> Optional[str]:
        """Poll ``getWaitingFlag`` and retry before giving up.

        Parameters
        ----------
        flag
            Flag expected from ``MooseControl``.
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
                    self.logger.info(
                        f"Successfully got flag '{result}' after {retries} retries."
                    )
                    return result
            except Exception as exc:  # pragma: no cover - defensive logging
                self.logger.warning(
                    f"Attempt {retries + 1}/{max_retries} failed: {exc}"
                )

            retries += 1
            self.logger.info(
                f"Waiting {wait_seconds} seconds before retrying..."
            )
            time.sleep(wait_seconds)

        self.logger.error(
            f"Failed to get flag '{flag}' after {max_retries} retries."
        )
        return None
