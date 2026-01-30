from moosefmu import Moose2FMU
from pythonfmu.default_experiment import DefaultExperiment
from pythonfmu.enums import Fmi2Causality, Fmi2Variability
from pythonfmu.variables import Real, String


class DomeTest(Moose2FMU):
    """Example FMU demonstrating the typical Moose2FMU integration flow."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # Public state variables exposed through FMU outputs so that the host
        # simulator can read values produced by the MOOSE application.
        self.moose_time: float = 0.0
        self.time: float = 0.0
        self._fast_forwarded = False

        self.mfr_in: float = 0.1
        self.mfr_out: float =  0.1
        self.T_in: float = 270
        self.T_out: float = 310
        self.T_air: float = 300
        self.reactor_power = 50e3

        self.air_heatrate = 0.0


        # Declare the FMU variable metadata.  Controllable inputs use
        # ``Fmi2Causality.input`` and outputs expose calculated MOOSE values.
        self.register_variable(
            Real(
                "mfr_in",
                causality=Fmi2Causality.input,
                variability=Fmi2Variability.continuous,
            )
        )

        self.register_variable(
            Real(
                "mfr_out",
                causality=Fmi2Causality.input,
                variability=Fmi2Variability.continuous,
            )
        )

        self.register_variable(
            Real(
                "T_in",
                causality=Fmi2Causality.input,
                variability=Fmi2Variability.continuous,
            )
        )

        self.register_variable(
            Real(
                "T_out",
                causality=Fmi2Causality.input,
                variability=Fmi2Variability.continuous,
            )
        )

        self.register_variable(
            Real(
                "T_air",
                causality=Fmi2Causality.input,
                variability=Fmi2Variability.continuous,
            )
        )

        self.register_variable(
            Real(
                "reactor_power",
                causality=Fmi2Causality.input,
                variability=Fmi2Variability.continuous,
            )
        )

        self.register_variable(
            Real(
                "air_heatrate",
                causality=Fmi2Causality.output,
                variability=Fmi2Variability.continuous,
            )
        )

        # FMU metadata that instructs the host about valid integration bounds.
        self.default_experiment = DefaultExperiment(
            start_time=0.0, stop_time=6000.0, step_size=2000
        )

        self.logger.info("MooseTest instance created.")

    def do_step(self, current_time: float, step_size: float) -> bool:
        """
        Advance the FMU by a single macro step.

        The body of ``do_step`` showcases the canonical Moose2FMU workflow:

        1. Optionally push controllable parameter updates to MOOSE using the
           ``set_controllable_*`` helpers.
        2. Synchronize time with the MooseControl server (which performs the
           heavy lifting of running the application).
        3. Read postprocessor and reporter data that become FMU outputs.
        Returning ``True`` signals to the master algorithm that the step
        completed successfully; ``False`` indicates a fatal integration error.
        """
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
            air_heatrate = self.get_postprocessor_value("air_heatrate", current_time)

            # If the value is not found, fail the step to surface the issue to
            # the master algorithm.
            if air_heatrate is None:
                return False

            # send value to FMU variable
            self.air_heatrate = air_heatrate


            self.logger.info("Information sync complete!")
            # resume MOOSE simulation after sync
            self.control.set_continue()

        return True
