from pythonfmu import Fmi2Causality, Fmi2Variability, Fmi2Slave, Real, String, Integer, Boolean
from pythonfmu.default_experiment import DefaultExperiment

class PythonSlave(Fmi2Slave):

    author = "John Doe"
    description = "A simple description"

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.intOut = 1
        self.realOut = 3.0

        self.booleanVariable = True
        self.stringVariable = "Hello World!"

        self.Kp     = 1.0
        self.in_val = 0.0

        self.register_variable(Integer("intOut", causality=Fmi2Causality.output, variability=Fmi2Variability.discrete))
        self.register_variable(Real("realOut", causality=Fmi2Causality.output))
        self.register_variable(
            Boolean("booleanVariable", causality=Fmi2Causality.local, variability=Fmi2Variability.discrete)
        )
        self.register_variable(String("stringVariable", causality=Fmi2Causality.local))

        self.register_variable(
            Real("Kp",
                 causality=Fmi2Causality.parameter,
                 variability=Fmi2Variability.fixed)
        )
        self.register_variable(
            Real("in_val",
                 causality=Fmi2Causality.input,
                 variability=Fmi2Variability.continuous)
        )

        self.default_experiment = DefaultExperiment(
            start_time=0.0,    # simulation begins at t=0 s
            stop_time=10.0,    # simulation ends at t=10 s
            step_size=self.Kp     # fixed time‐step of 0.01 s
        )


    def do_step(self, current_time, step_size):
        u = self.in_val

        # read the fixed parameter
        k = self.Kp

        # compute your output however you like
        self.realOut = k * u
        self.intOut = k

        print(f"current_time is {current_time}, time_step size is {step_size}")

        if self.realOut == 7.5:
            return False

        return True
