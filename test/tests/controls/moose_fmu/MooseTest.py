from pythonfmu import Fmi2Slave
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Integer, Real, String
from MooseControl import MooseControl
from pythonfmu.default_experiment import DefaultExperiment
from MooseFMU import Moose2FMU
import logging

class MooseTest(Moose2FMU):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # State variables specific to the DemoSlave
        self.moose_time: float = 0.0
        self.time: float = 0.0
        self.diffused: float = 0.0
        self._fast_forwarded = False
        self.BC_info: str = ''
        self.BC_value: float = 0.0
        self.rep_value: float = 0.0

        self.register_variable(Real("diffused", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))
        self.register_variable(Real("rep_value", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))
        self.register_variable(String("BC_info", causality=Fmi2Causality.input, variability=Fmi2Variability.discrete))
        self.register_variable(Real("BC_value", causality=Fmi2Causality.input, variability=Fmi2Variability.continuous))

        # Default experiment configuration
        self.default_experiment = DefaultExperiment(start_time=0.0, stop_time=3.0, step_size=0.1)

        self.logger.info("MooseTest instance created.")

    def do_step(self,
                current_time: float,
                step_size:    float) -> bool:

        # Set a controllable ``Real`` parameter as boundary condition.
        if self.BC_info:
            if self.set_controllable_real(self.BC_info, self.BC_value):
                self.logger.info(
                    f"Change boundary condition {self.BC_info} to {self.BC_value}")

        # Synchronize MOOSE simulation time with FMU (support MOOSE simulation time stepping mechanism)
        moose_time, signal = self.sync_with_moose(current_time, step_size, self.flag)

        if moose_time is None:
                return False

        # Verify the MooseControl server is listening before proceeding
        if not self.ensure_control_listening():
            return False

        self.logger.info("Information exchange")
        self.moose_time = moose_time
        self.time = current_time

        # MOOSE time is synced with FMU time, we will get the value needed from MOOSE
        if signal:
            # get the postprocessor value named "diffused" from MOOSE
            diffused = self.get_postprocessor_value(self.flag, "diffused", current_time)

            # if the value is not found, fail the step
            if diffused is None:
                return False

            # send value to FMU variable
            self.diffused = diffused

            # get the value of "pi" reporter named "constant" from MOOSE
            rep_value = self.get_reporter_value(self.flag, "constant/pi", current_time)

            if rep_value is None:
                return False

            # send value to FMU variable
            self.rep_value = rep_value

        return True

