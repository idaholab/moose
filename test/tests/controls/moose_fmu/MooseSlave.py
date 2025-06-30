from pythonfmu import Fmi2Slave
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Boolean, Integer, Real, ScalarVariable, String
from MooseControl import MooseControl
from pythonfmu.fmi2slave import Fmi2Status
import requests
from pythonfmu.default_experiment import DefaultExperiment

class MooseSlave(Fmi2Slave):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs, logging_add_standard_categories=False)
        # Register one input and one output

        # self.control = MooseControl(
        #     moose_command=['../../../moose_test-opt','-i','get_postprocessor.i'],
        #     moose_control_name='Controls/web_server'
        # )
        self.control = MooseControl(moose_port=40000)


        self.moose_time = 0
        self.time = 0.0
        self.diffused = 0.0

        self.register_variable(Real("moose_time", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))
        self.register_variable(Real("time", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))
        self.register_variable(Real("diffused", causality=Fmi2Causality.output, variability=Fmi2Variability.continuous))


        self.default_experiment = DefaultExperiment(
        start_time=0.0,    # simulation begins at t=0 s
        stop_time=3.0,    # simulation ends at t=10 s
        step_size=0.5     # fixed time‐step, can not be adjusted after fmu is built
        )

        self.sim_start_time = None


    def setup_experiment(self,
                         start_time: float,
                         tolerance:  float = None,
                         stop_time:  float = None) -> bool:

        self.sim_start_time = start_time

        return True


    def do_step(self,
                current_time: float,
                step_size:    float,
                no_set_fmu_state_prior: bool = False) -> bool:

        # on first step, initialize MOOSE
        if current_time == self.sim_start_time:
            self.control.initialize()

        # if MOOSE isn't listening, shut down immediately
        if not self.control.isListening():
            self.control.finalize()
            print("MOOSE is not listening")
            return False

        # try to get the next waiting flag, with retries
        max_retries   = 3        # how many times to poll
        wait_for_flag = 'TIMESTEP_BEGIN'
        flag = None

        for attempt in range(1, max_retries+1):
            flag = self.control.getWaitingFlag()
            if flag:
                break
            print(f"[Attempt {attempt}/{max_retries}] no waiting flag yet, retrying…")
            # give MOOSE a chance to advance and set the flag
            # here we wait specifically for the TIMESTEP_BEGIN event
            self.control.wait(wait_for_flag)

        # if we still didn’t get any flag, abort cleanly
        if not flag:
            print(f"Failed to getWaitingFlag() after {max_retries} retries")
            self.control.finalize()
            return False

        # only handle TIMESTEP_BEGIN in this example
        if flag == wait_for_flag:
            # now safe to pull out the postprocessor values
            self.control.wait(flag)
            t_val = self.control.getPostprocessor('t')
            diffused_val = self.control.getPostprocessor('diffused')
            self.moose_time = t_val
            self.time       = current_time
            self.diffused = diffused_val

            # signal MOOSE to continue
            self.control.setContinue()

        return True
