from pythonfmu import Fmi2Slave, Fmi2Causality, Real
import time
from moosecontrol import MooseControl  # Assumed to be available from MOOSE


class MooseFMU(Fmi2Slave):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        # Define FMU interface variables
        self.boundary_temp = 300.0  # Input: e.g., ambient or boundary temperature [K]
        self.flange_avg_temp = 300.0  # Output: e.g., computed average temperature [K]
        self.register_variable(Real("boundary_temp", causality=Fmi2Causality.input))
        self.register_variable(Real("flange_avg_temp", causality=Fmi2Causality.output))
        self.current_time = 0.0

        # Connect to the running MOOSE instance via the MooseControl API.
        # Ensure that MOOSE is running with its web server enabled (default port 8080 here).
        self.moose = MooseControl("http://localhost:8080")
        self.moose.initialize()  # Optional: perform any initial handshake/setup
        print("MooseFMU: Connected to MOOSE via MooseControl.")

    def do_step(self, current_time, step_size):
        """
        Advance the MOOSE simulation from current_time to (current_time + step_size) by:
          1. Updating the controllable input(s) via the MooseControl API.
          2. Instructing MOOSE to advance its simulation.
          3. Polling until the simulation reaches the target time.
          4. Retrieving output (postprocessor) values.
        """
        self.current_time = current_time
        next_time = current_time + step_size

        # 1. Set the boundary temperature in the MOOSE simulation.
        self.moose.setControllableReal("boundary_temp", self.boundary_temp)
        print(
            f"MooseFMU: Set 'boundary_temp' to {self.boundary_temp} at t={current_time}."
        )

        # 2. Instruct MOOSE to advance simulation to the target time.
        # The advanceToTime method is assumed to trigger simulation up to 'next_time'.
        self.moose.advanceToTime(next_time)
        print(f"MooseFMU: Advancing simulation to t={next_time}.")

        # 3. Wait (poll) until the MOOSE simulation reaches the target time.
        timeout = 60  # seconds timeout for simulation step
        start_wait = time.time()
        while True:
            sim_time = self.moose.getCurrentTime()
            if sim_time >= next_time:
                break
            if time.time() - start_wait > timeout:
                print(
                    "MooseFMU: Timeout while waiting for simulation to reach target time."
                )
                return False
            time.sleep(0.1)  # poll every 100 ms

        # 4. Retrieve the output value from the designated postprocessor.
        self.flange_avg_temp = self.moose.getPostprocessorValue("flange_avg_temp")
        print(
            f"MooseFMU: Retrieved 'flange_avg_temp' = {self.flange_avg_temp} at t={next_time}."
        )
        return True

    def terminate(self):
        """
        Cleanly terminate the connection with MOOSE. This may include shutting down
        the control interface or performing other cleanup tasks.
        """
        self.moose.finalize()
        print("MooseFMU: Terminated connection with MOOSE.")


# Standalone test code (optional)
if __name__ == "__main__":
    # Create an instance of the FMU
    fmu_instance = MooseFMU()

    # Simulate 10 steps of 1 second each
    current_time = 0.0
    step_size = 1.0
    for step in range(10):
        print(f"\n--- FMU Step {step + 1} ---")
        # Optionally, update the input (boundary_temp) for demonstration purposes
        fmu_instance.boundary_temp = 300.0 + step * 5.0
        if not fmu_instance.do_step(current_time, step_size):
            print("Error during simulation step.")
            break
        current_time += step_size
        print(f"Time: {current_time}, Flange Avg Temp: {fmu_instance.flange_avg_temp}")

    # Terminate the FMU simulation session
    fmu_instance.terminate()
