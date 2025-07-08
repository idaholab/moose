from pythonfmu import Fmi2Slave
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Real
from MooseControl import MooseControl
from pythonfmu.default_experiment import DefaultExperiment
from MOOSE2FMU import MooseSlave

class MooseDemo(MooseSlave):
    """
    Demo FMU slave using MooseSlave as base. Implements the do_step logic and sets a default experiment.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # State variables specific to the DemoSlave
        self.moose_time: float = 0.0
        self.time: float = 0.0
        self.diffused: float = 0.0

        self.register_variable(Real("diffused", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))

        # Default experiment configuration
        self.default_experiment = DefaultExperiment(start_time=0.0, stop_time=3.0, step_size=0.5)


    def do_step(
        self,
        current_time: float,
        step_size: float,
        no_set_fmu_state_prior: bool = False,
    ) -> bool:

        # Abort if MOOSE is unresponsive
        if not self.control.isListening():
            self.control.finalize()
            print("MOOSE is not listening")
            return False

        # Poll for the desired flag
        flag = self._get_flag_with_retries(self.flag, self.max_retries)
        if not flag:
            print(f"Failed to getWaitingFlag() after {self.max_retries} retries")
            self.control.finalize()
            return False

        # When the expected flag arrives, read postprocessor values
        if flag == self.flag:
            self.control.wait(flag)
            t_val = self.control.getPostprocessor("t")
            diffused_val = self.control.getPostprocessor("diffused")
            self.moose_time = t_val
            self.time = current_time
            self.diffused = diffused_val
            self.control.setContinue()
        else:
            # Unexpected flag, just continue
            print(f"Received unexpected flag: {flag}")
            self.control.setContinue()

        return True
