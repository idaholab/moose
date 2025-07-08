from pythonfmu import Fmi2Slave
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Boolean, Integer, Real, ScalarVariable, String
from MooseControl import MooseControl
from pythonfmu.default_experiment import DefaultExperiment
from typing import Optional
from MOOSE2FMU import MooseSlave
import time
import logging

#logging.disable(logging.CRITICAL)

class MooseTest(MooseSlave):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.logger.info("MooseTest instance created.")
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
        self.default_experiment = DefaultExperiment(start_time=0.0, stop_time=3.0, step_size=0.1)

    def do_step(self,
                current_time: float,
                step_size:    float,
                no_set_fmu_state_prior: bool = False) -> bool:



        self.logger.info(f"The change boundary flag is {self.change_BC}")
        if self.change_BC:
             self.logger.info(f"Change boundary condition {self.BC_info} to {self.BC_value}")
             self.control.setControllableReal(self.BC_info, self.BC_value)
             self.change_BC = False


        while True:
            flag = self._get_flag_with_retries(self.flag, self.max_retries)
            if not flag:
                self.logger.error(f"Failed to fast-forward to {current_time}")
                self.control.finalize()
                return False
            self.control.wait(flag)
            t_val = self.control.getPostprocessor("t")
            diffused = self.control.getPostprocessor("diffused")
            self.control.setContinue()

            if abs(t_val - current_time) < self.dt_tolerance:
                break

        # Now do a normal FMU step
        if not self.control.isListening():
            print("MOOSE is not listening")
            self.control.finalize()
            return False

        self.logger.info("Write out results")
        self.moose_time = t_val
        self.time = current_time
        self.diffused = diffused
        self.logger.info(f"The current time is {self.time}, the moose time is {self.moose_time}")

        return True


    def _get_flag_with_retries(self, flag: str, max_retries: int, wait_seconds=0.5) -> str | None:
        retries = 0
        while retries < max_retries:
            try:
                result = self.control.getWaitingFlag()
                if result:
                    self.logger.info(f"Successfully got flag '{result}' after {retries} retries.")
                    return result
            except Exception as e:
                self.logger.warning(f"Attempt {retries+1}/{max_retries} failed: {e}")

            retries += 1
            self.logger.info(f"Waiting {wait_seconds} seconds before retrying...")
            time.sleep(wait_seconds)  # Pauses the loop for specified seconds

        self.logger.error(f"Failed to get flag '{result}' after {max_retries} retries.")

        return None
