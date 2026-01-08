# StochasticControl -- Python Interface for MOOSE Stochastic Tools

A lightweight wrapper that builds a stochastic MOOSE input, runs samples, and returns quantities of interest (QoIs) as NumPy arrays.

## Introduction

- Assembles a MOOSE input for stochastic sampling (sampler, multiapp, transfers/reporters).
- Launches the MOOSE executable (optionally via MPI) and coordinates sample execution.
- Returns QoIs in shapes that match your inputs.

## Quick Start

Ensure that the following paths are in you `PYTHONPATH`:

- `/path/to/moose/python`
- `/path/to/moose/modules/stochastic_tools/python`

```python
import numpy as np
from moose_stochastic_tools import StochasticControl, StochasticRunOptions

executable = "/path/to/moose-app-opt"     # MOOSE app with stochastic_tools
physics_input = "physics.i"               # Your base physics input
parameters = ["Materials/K/value",
              "Kernels/convection/alpha",
              "beta"]                     # Parameters to sample (names as seen by the app)
qois = ["pp/value"]                       # Reporter(s) to collect as QoIs

# Configure how the stochastic run is executed
opts = StochasticRunOptions(
    num_procs=4,                   # mpiexec -n 4 ...
    mpi_command="mpiexec",
    input_name="stochastic_run.i", # name of created stochastic tools input (optional)
    cli_args=[],                   # extra CLI args to the *stochastic* driver input
    min_procs_per_sample=None,     # per-sample parallelism (optional)
    multiapp_mode=StochasticRunOptions.MultiAppMode.BATCH_RESET,
    ignore_solve_not_converge=False
)

# Optionally, pass CLI args to the *physics* app
physics_cli_args = []

with StochasticControl(executable, physics_input, parameters, qois,
                       options=opts, physics_cli_args=physics_cli_args) as runner:
    # x: N x P matrix (N samples, P parameters)
    x = np.array([
        [1.0, 0.1, 0.2],
        [1.1, 0.1, 0.25],
        [0.9, 0.12, 0.2],
    ])
    y = runner(x)   # -> shape (N, Q, ...) if multiple QoIs, vector if one QoI
    print(y)
```

What happens under the hood:

- First call will write a generated file (default `stochastic_run.i`) and start the app; subsequent calls continue the run.
- Utilizes the [WebServerControl](WebServerControl.md) to control an [InputMatrix](InputMatrixSampler.md) sampler, which is performed each timestep in the simulation.
- Either a [MultiAppSamplerControl](MultiAppSamplerControl.md) or [SamplerParameterTransfer](SamplerParameterTransfer.md), depending on the multi-app mode, is used to set the parameters in the sub-application.
- QoIs are pulled from reporter values via a [StochasticMatrix](StochasticMatrix.md) and [SamplerReporterTransfer](SamplerReporterTransfer.md).
- Simulation is terminated gracefully once exited from the context manager (or killed if exception is raised).

## Details

### Shapes and Return Types

`runner(x)` returns:

- float if you pass a single row and have a single QoI,
- 1-D array if one of {samples, QoIs} is singular,
- 2-D array (N×Q) for reporters that return scalar QoIs.
- N-D array (N×Qx?x...) otherwise.

### CLI Arguments

- Stochastic driver CLI: set via `options.cli_args` and appended to the top-level command (`<mpi> <exe> -i <input_name> ...`). Useful for things like specifying an output of all the runs, such as `Outputs/json=true`.
- Physics app CLI: set via the `physics_cli_args` argument. Useful when there are multiple stochastic runs with different fixed parameters in the physics input.

### MultiApp Modes

Five options for the how the stochastic run is executed:

- `NORMAL`
- `BATCH_RESET`
- `BATCH_RESTORE`
- `BATCH_KEEP_SOLUTION`
- `BATCH_NO_RESTORE`

These are analogous to the modes described in [ParameterStudy](ParameterStudy/index.md).

## Tips

- Enable the cache if you expect repeated or near-repeated rows. Example:

  ```python
  with StochasticControl(...) as runner:
      runner.configCache()

      y1 = runner([0, 1, 2])              # Runs STM input for the first time
      y2 = runner([0, 1, 2])              # Does not run input and returns y1
      y3 = runner([[0, 1, 2], [3, 4, 5]]) # Only runs input for [3, 4, 5]
  ```

- Set `min_procs_per_sample` in `StochasticRunOptions` if each sample itself needs multiple processes (e.g., heavy physics solves requiring a lot of memory).

## API Reference

!alert! construction

The following requires MooseDocs to add `stochastic_tools` python directory into the system path:

```markdown
!pysyntax class name=moose_stochastic_tools.StochasticRunOptions heading-level=3 show-internal=False show-private=False show-protected=False

!pysyntax class name=moose_stochastic_tools.StochasticControl heading-level=3 show-internal=False show-private=False show-protected=False

!pysyntax class name=moose_stochastic_tools.StochasticControl.StochasticRunner heading-level=3 show-internal=False show-private=False show-protected=False
```

!alert-end!
