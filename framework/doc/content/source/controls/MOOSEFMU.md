# MOOSE FMU Interface

The `MOOSEFMU` defines the `Moose2FMU` base class which contains the boilerplate needed to wrap a MOOSE simulation as a Functional Mock-up Unit (FMU). Users only need to implement their own `__init__` and `do_step` methods when deriving from `Moose2FMU`.

### Initialization Parameters

`Moose2FMU` accepts a number of keyword arguments that configure how the
underlying MOOSE simulation is launched and interacted with:

- `flag`: Optional flag that is forwarded to the MOOSE input deck.
- `moose_mpi` / `mpi_num`: Configure MPI launching when running distributed
  MOOSE executables.
- `moose_executable` / `moose_inputfile`: Path to the simulation binary and the
  input file that should be executed inside the FMU.
- `server_name`: Name of the `MooseControl` server block to connect to.
- `max_retries`: Number of times helper utilities will poll the control server
  before aborting.
- `dt_tolerance`: Permitted synchronization tolerance between FMU time and the
  MOOSE simulation clock.

The class also exposes a ``default_experiment`` that can be customized if the
FMU should advertise a different start/stop time or nominal step size.


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

### Building the FMU

Save the custom class in a Python file and use [`pythonfmu`](https://github.com/NTNU-IHB/PythonFMU) to build the FMU:

```bash
pythonfmu build custom_moose.py
```

The resulting `.fmu` file can then be imported into any compliant co-simulation environment.

### Testing the FMU

Sample helper scripts in `test/tests/controls/moose_fmu` demonstrate how to exercise a
generated FMU with [`fmpy`](https://github.com/CATIA-Systems/FMPy):

- `run_fmu.py` runs `MooseTest.fmu` in a stand-alone mode using `simulate_fmu` and
  prints the time, `moose_time` and `diffused` outputs.
- `run_fmu_connection.py` shows how to couple `MooseTest.fmu` with another FMU (e.g.,
  `Dahlquist.fmu`) and change boundary conditions during the simulation.

To try the examples:

```bash
cd test/tests/controls/moose_fmu
python run_fmu.py               # stand‑alone Moose FMU
python run_fmu_connection.py    # coupled FMUs example
```
