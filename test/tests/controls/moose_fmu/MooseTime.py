from pythonfmu import Fmi2Slave
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Integer, Real, String
from MooseControl import MooseControl
from pythonfmu.default_experiment import DefaultExperiment
from MooseFMU import Moose2FMU
import logging

class MooseTime(Moose2FMU):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # State variables specific to the DemoSlave
        self.moose_time: float = 0.0
        self.time: float = 0.0

        self._fast_forwarded = False
        self.BC_info: str = ''
        self.BC_value: float = 0.0
        self.rep_value: float = 0.0

        # Default experiment configuration
        self.default_experiment = DefaultExperiment(start_time=0.0, stop_time=3.0, step_size=0.1)

        self.logger.info("MooseTest instance created.")

    def do_step(self,
                current_time: float,
                step_size:    float) -> bool:

        # Synchronize MOOSE simulation time with FMU (support MOOSE simulation time stepping mechanism)
        moose_time, signal = self.sync_with_moose(current_time, step_size, self.flag)

        if moose_time is None:
                return False

        # Verify the MooseControl server is listening before proceeding
        if not self.ensure_control_listening():
            return False

        if signal:
            self.logger.info("Information exchange")
            self.moose_time = moose_time
            self.time = current_time


        return True

