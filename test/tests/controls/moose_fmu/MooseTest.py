from pythonfmu import Fmi2Slave
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Boolean, Integer, Real, ScalarVariable, String
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
        self.change_BC: bool = False

        self.register_variable(Real("diffused", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))
        self.register_variable(Boolean("change_BC", causality=Fmi2Causality.input, variability=Fmi2Variability.discrete))
        self.register_variable(String("BC_info", causality=Fmi2Causality.input, variability=Fmi2Variability.discrete))
        self.register_variable(Real("BC_value", causality=Fmi2Causality.input, variability=Fmi2Variability.continuous))
        # Default experiment configuration
        self.default_experiment = DefaultExperiment(start_time=0.0, stop_time=3.0, step_size=0.5)
        self.logger.info("MooseTest instance created.")

    def do_step(self,
                current_time: float,
                step_size:    float,
                no_set_fmu_state_prior: bool = False) -> bool:

        # self.logger.info(f"The change boundary flag is {self.change_BC}")
        # if self.change_BC:
        #     self.logger.info(f"Change boundary condition {self.BC_info} to {self.BC_value}")
        #     self.control.setControllableReal(self.BC_info, self.BC_value)
        #     self.change_BC = False
        #     self.control.setContinue()

        moose_time, signal = self._sync_with_moose(current_time, self.flag)

        if moose_time is None:
                return False

        if not self._ensure_control_listening():
            return False

        self.logger.info("Write out results")
        self.moose_time = moose_time
        self.time = current_time

        # MOOSE time is synced with FMU time, we will get the value needed from MOOSE
        if signal:
            flag = self._get_flag_with_retries(self.flag, self.max_retries)

            if not flag:
                self.logger.error(f"Failed to fast-forward to {current_time}")
                self.control.finalize()
                return False

            self.control.wait(flag)
            diffused = self.control.getPostprocessor("diffused")
            self.control.setContinue()
            self.logger.info(
                f"Retrieved Postprocessor value {diffused} from MOOSE at flag {flag}")
            self.diffused = diffused

        return True

