from pythonfmu3 import Fmi3Causality, Fmi3Variability, Fmi3Slave, Float64, String, Int32, Boolean
from pythonfmu3.default_experiment import DefaultExperiment
from pythonfmu3 import Fmi3Status, Fmi3StepResult

class PythonSlave(Fmi3Slave):

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

        self.register_variable(Int32("intOut", causality=Fmi3Causality.output, variability=Fmi3Variability.discrete))
        self.register_variable(Float64("realOut", causality=Fmi3Causality.output))
        self.register_variable(
            Boolean("booleanVariable", causality=Fmi3Causality.local, variability=Fmi3Variability.discrete)
        )
        self.register_variable(String("stringVariable", causality=Fmi3Causality.local))

        self.register_variable(
            Float64("Kp",
                 causality=Fmi3Causality.parameter,
                 variability=Fmi3Variability.fixed)
        )
        self.register_variable(
            Float64("in_val",
                 causality=Fmi3Causality.input,
                 variability=Fmi3Variability.continuous)
        )

        self.default_experiment = DefaultExperiment(
            start_time=0.0,    # simulation begins at t=0 s
            stop_time=10.0,    # simulation ends at t=10 s
            step_size=0.5     # fixed time‐step of 0.01 s
        )

        # Note:
        # it is also possible to explicitly define getters and setters as lambdas in case the variable is not backed by a Python field.
        # self.register_variable(Real("myReal", causality=Fmi3Causality.output, getter=lambda: self.realOut, setter=lambda v: set_real_out(v))

    def do_step(self, current_time, step_size):
        u = self.in_val

        # read the fixed parameter
        k = self.Kp

        # compute your output however you like
        self.realOut = k * u
        self.intOut = k

        print(f"current_time is {current_time}, time_step size is {step_size}")

        if self.realOut == 7.5:
            return Fmi3StepResult(status=Fmi3Status.ok,
                        terminateSimulation=True)

        return Fmi3StepResult(status=Fmi3Status.ok)

    def getBooleanStatus(self, statusKind: int) -> bool:
        # 0 is fmi2DoStepStatus → treat any step as a 'failure' to stop early
        print("Am I here?")
        if statusKind == 0:
            return False
        # fall back to the default behavior for other status-kinds
        return super().getBooleanStatus(statusKind)
