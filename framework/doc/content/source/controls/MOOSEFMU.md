# MOOSE FMU Interface

The `MOOSEFMU` defines the `Moose2FMU` base class which contains the boilerplate needed to wrap a MOOSE simulation as a Functional Mock-up Unit (FMU). Users only need to implement their own `__init__` and `do_step` methods when deriving from `Moose2FMU`.

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

- `run_server_fmu.py` runs `MooseTest.fmu` in a stand-alone mode using `simulate_fmu` and
  prints the time, `moose_time` and `diffused` outputs.
- `run_fmu_connection.py` shows how to couple `MooseTest.fmu` with another FMU (e.g.,
  `Dahlquist.fmu`) and change boundary conditions during the simulation.

To try the examples:

```bash
cd test/tests/controls/moose_fmu
python run_server_fmu.py        # stand‑alone Moose FMU
python run_fmu_connection.py    # coupled FMUs example
```
