# MOOSE FMU Interface

The `MOOSEFMU` defines the `Moose2FMU` Python base class which contains the boilerplate needed to wrap a MOOSE simulation as a Functional Mock-up Unit (FMU) using the FMI 2.0 standard. Users only need to implement their own `__init__` and `do_step` methods when deriving from `Moose2FMU`.

### Initialization Parameters

`Moose2FMU` accepts a handful of keyword arguments that configure how the
underlying MOOSE simulation is launched and synchronized. The example FMU tests
in `test/tests/controls/moose_fmu` pass these parameters as FMI start values, so
you can see them in action in `run_fmu.py` and the `simulate_moose_fmu` helper
inside `moose_fmu_tester.py`:

- `flag`: Optional user-defined synchronization flag that supplements the
  default `INITIAL`, `MULTIAPP_FIXED_POINT_BEGIN`, and `MULTIAPP_FIXED_POINT_END`
  signals.
- `moose_command`: Full command string used to launch the MOOSE executable and
  input file (for example, including MPI launchers when needed).
- `server_name`: Name of the `MooseControl` server block to connect to.
- `max_retries`: Number of times helper utilities will poll the control server
  before aborting.
- `dt_tolerance`: Permitted synchronization tolerance between FMU time and the
  MOOSE simulation clock.

!listing python/MooseFMU/MOOSE2FMU.py start=def __init__ end=# Default synchronization and data retrieval flags

The class also exposes a ``default_experiment`` that can be customized if the
FMU should advertise a different start/stop time or nominal step size.


### Runtime helpers

`Moose2FMU` provides a small collection of convenience methods that simplify
interacting with a running MOOSE simulation:

- `set_controllable_real` and `set_controllable_vector` push new controllable
  values to the `WebServerControl` system. Values are cached to avoid
  redundant updates; pass ``force=True`` to resend a value that was already
  applied.
- `sync_with_moose` advances the FMU time until it matches the simulation time
  within `dt_tolerance`. Additional synchronization flags can be supplied via
  the optional `allowed_flags` argument.
- `ensure_control_listening` verifies the control socket is ready before any
  postprocessors or reporters are queried.
- `get_postprocessor_value` and `get_reporter_value` wait for a specific
  synchronization flag and return the requested quantity.
- `get_flag_with_retries` normalizes retry logic when polling MOOSE for the
  next synchronization flag.

Together these helpers reduce the boilerplate needed inside a custom
`do_step` implementation and make it easier to write robust coupling logic.

### Synchronization and data retrieval flags

`Moose2FMU` coordinates with the running MOOSE simulation through named
`MooseControl` flags. To simplify typical multiapp workflows the base class
waits for a set of synchronization signals by default:

- `INITIAL` and `MULTIAPP_FIXED_POINT_BEGIN` are consumed before attempting to
  advance the coupled simulation. `MULTIAPP_FIXED_POINT_BEGIN` is the first flag
  raised immediately after the new time step has been computed, so listening
  for it gives the FMU the earliest opportunity to synchronize clocks.
- `MULTIAPP_FIXED_POINT_END` is consumed before any reporter or postprocessor
  values are retrieved. In a typical execution order controls run ahead of
  reporters and postprocessors, and those objects usually execute at
  `TIMESTEP_END`. The `MULTIAPP_FIXED_POINT_END` flag fires right after
  `TIMESTEP_END`, ensuring the data pulled from MOOSE reflects the updated state
  of the current time step.

Any flag string supplied through the `flag` initialization argument is merged
into these defaults, and the higher-level helpers such as
`set_controllable_real`, `set_controllable_vector`, `get_reporter_value`, and
`get_postprocessor_value` also accept per-call `flag` arguments. Passing custom
flags in these locations allows FMUs to react to user-defined synchronization
points while still benefiting from the built-in defaults. Refer to
[SetupInterface.md] for additional details on
MOOSE execute flags.

### Creating a Custom FMU

```python
from MooseFMU import Moose2FMU


class CustomMoose(Moose2FMU):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        # register additional inputs/outputs here

    def do_step(
        self,
        current_time: float,
        step_size: float,
        no_set_fmu_state_prior: bool = False,
    ) -> bool:
        # advance the MOOSE simulation and update outputs
        return True
```

A typical step method will synchronize with MOOSE and make use of the helper
utilities described above:

```python
    def do_step(self, current_time: float, step_size: float, **_kwargs) -> bool:
        moose_time, signal = self.sync_with_moose(current_time, step_size)
        if moose_time is None:
            return False

        if signal == "MULTIAPP_FIXED_POINT_END":
            diffused = self.get_postprocessor_value(signal, "diffused", current_time)
            if diffused is None:
                return False
            self.diffused = diffused

        self.set_controllable_real("BCs/boundary", 1.0)
        return True
```

### Building the FMU

Save the custom class in a Python file and use [`pythonfmu`](https://github.com/NTNU-IHB/PythonFMU) to build the FMU:

```bash
pythonfmu build MooseTest.py
```

The resulting `.fmu` file can then be imported into any standard-compliant co-simulation environment.

### Testing the FMU

Sample helper scripts in `test/tests/controls/moose_fmu` demonstrate how to exercise a
generated FMU with [`fmpy`](https://github.com/CATIA-Systems/FMPy)and show the
initialization parameters in context:

- `run_fmu.py` runs `MooseTest.fmu` in a stand-alone mode using `simulate_fmu` and
  prints the time, `moose_time` and `diffused` outputs.
- `run_fmu_connection.py` shows how to couple `MooseTest.fmu` with another FMU (e.g.,
  `Dahlquist.fmu`) and change boundary conditions during the simulation.
- `run_fmu_step_by_step.py` walks through the FMU initialization sequence
  (`instantiate → setupExperiment → enterInitializationMode → apply_start_values →
  exitInitializationMode`), making it clear where each start value is applied.

To try the examples:

```bash
cd test/tests/controls/moose_fmu
python run_fmu.py               # stand‑alone Moose FMU
python run_fmu_connection.py    # coupled FMUs example
```

