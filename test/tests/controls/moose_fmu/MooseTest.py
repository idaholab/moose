from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Real, String
from pythonfmu.default_experiment import DefaultExperiment
from MooseFMU import Moose2FMU

class MooseTest(Moose2FMU):
    """Example FMU demonstrating the typical Moose2FMU integration flow.

    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # Public state variables exposed through FMU outputs so that the host
        # simulator can read values produced by the MOOSE application.
        self.moose_time: float = 0.0
        self.time: float = 0.0
        self.diffused: float = 0.0
        self._fast_forwarded = False
        self.BC_info: str = ""
        self.BC_value: float = 0.0
        self.rep_value: float = 0.0

        # Declare the FMU variable metadata.  Controllable inputs use
        # ``Fmi2Causality.input`` and outputs expose calculated MOOSE values.
        self.register_variable(
            Real(
                "diffused",
                causality=Fmi2Causality.output,
                variability=Fmi2Variability.continuous,
            )
        )
        self.register_variable(
            Real(
                "rep_value",
                causality=Fmi2Causality.output,
                variability=Fmi2Variability.continuous,
            )
        )
        self.register_variable(
            String(
                "BC_info",
                causality=Fmi2Causality.input,
                variability=Fmi2Variability.discrete,
            )
        )
        self.register_variable(
            Real(
                "BC_value",
                causality=Fmi2Causality.input,
                variability=Fmi2Variability.continuous,
            )
        )

        # FMU metadata that instructs the host about valid integration bounds.
        self.default_experiment = DefaultExperiment(
            start_time=0.0, stop_time=2.0, step_size=0.5
        )

        self.logger.info("MooseTest instance created.")

    def do_step(self,
                current_time: float,
                step_size:    float) -> bool:
        """Advance the FMU by a single macro step.

        The body of ``do_step`` showcases the canonical Moose2FMU workflow:

        1. Optionally push controllable parameter updates to MOOSE using the
           ``set_controllable_*`` helpers.
        2. Synchronize time with the MooseControl server (which performs the
           heavy lifting of running the application).
        3. Read postprocessor and reporter data that become FMU outputs.
        Returning ``True`` signals to the master algorithm that the step
        completed successfully; ``False`` indicates a fatal integration error.
        """

        # Set a controllable ``Real`` parameter as boundary condition whenever
        # the host provides both the name and value of a target controllable.
        if self.BC_info:
            if self.set_controllable_real(self.BC_info, self.BC_value):
                self.logger.info(
                    f"Change boundary condition {self.BC_info} to {self.BC_value}")

        # Synchronize MOOSE simulation time with the FMU (supports MOOSE
        # simulation time stepping mechanism).
        moose_time, signal = self.sync_with_moose(current_time, step_size, self.flag)

        if moose_time is None:
            return False

        # Verify the MooseControl server is listening before proceeding.
        if not self.ensure_control_listening():
            return False

        self.moose_time = moose_time
        self.time = current_time

        # MOOSE time is synced with FMU time, we will get the value needed from
        # MOOSE when a fixed-point signal arrives from the control server.
        if signal:
            # get the postprocessor value named "diffused" from MOOSE.
            diffused = self.get_postprocessor_value("diffused", current_time)

            # If the value is not found, fail the step to surface the issue to
            # the master algorithm.
            if diffused is None:
                return False

            # send value to FMU variable
            self.diffused = diffused

            # get the value of "pi" reporter named "constant" from MOOSE.
            rep_value = self.get_reporter_value("constant/pi", current_time)

            if rep_value is None:
                return False

            # send value to FMU variable.
            self.rep_value = rep_value

            self.logger.info("Information sync complete!")
            # resume MOOSE simulation after sync
            self.control.setContinue()



        return True

