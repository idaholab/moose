# MooseControl

A Python class for interacting with a simulation running with a [WebServerControl.md], which exposes a REST API for changing the simulation. It can optionally also spawn and maintain a local MOOSE process along side your custom Python logic.

The general workflow is as follows:

1. Spawn a MOOSE process that has a listening [WebServerControl.md] object. This can be maintained by the `MooseControl` if utilizing one of the subprocess runners (see below).
1. Wait for the MOOSE process to be listening. Done by `MooseControl.initialize()` or automatically on enter if using the `MooseControl` as a Python context manager.
1. Wait for the MOOSE process to be waiting for external input within the [WebServerControl.md]. Done by `MooseControl.wait()`, which will poll for MOOSE to be waiting.
1. Interact with the MOOSE process: reading values or setting values. Done by the various methods in `MooseControl`.
1. Tell the MOOSE process to continue with its solve. Done by `MooseControl.set_continue()`.
1. Repeat steps 3-5 until the simulation is done, or use `MooseControl.set_terminate()` to tell the simulation to terminate early.
1. Wait for the MOOSE process to cleanup (finish running and listening). Done by `MooseControl.finalize()`, or automatically on exit of using the `MooseControl` as a Python context manager.

## Runners

A runner describes to the `MooseControl` how to connect to the running MOOSE process. You must first create a runner before creating the `MooseControl`. The following runners exist:

- `PortRunner`: Connect to a running MOOSE process over a port.
- `SocketRunner`: Connect to a running MOOSE process over a socket.
- `SubprocessPortRunner`: Spawn a MOOSE process given a command and connect to it over a port.
- `SubprocessSocketRunner`: Spawn a MOOSE process given a command and connect to it over a socket.

If interacting with the `MooseControl` for the first time, it is best to use either `SubprocessPortRunner` or `SubprocessSocketRunner`, as these runners systematically maintain the running MOOSE process for you.

All runners share the following optional arguments:

- `poll_time`: How often to poll the MOOSE simulation when waiting for it to do something in seconds. A common workflow will involve waiting for a MOOSE simulation to do something. This poll time is how often to check if the simulation is done and waiting for input.
- `poke_poll_time`: How often in seconds to "poke" the running MOOSE executable. The [WebServerControl.md] has two timeout parameters, [!param](/Controls/WebServerControl/initial_client_timeout) and [!param](/Controls/WebServerControl/client_timeout), which require that the MOOSE simulation hears from the client every so often otherwise it will error. This ensures that the MOOSE simulation does not sit around indefinitely as a zombie waiting for a message from the control.
- `initialize_timeout`: How long in seconds to wait for the MOOSE simulation to start listening. If this timeout is hit, the `MooseControl` will raise an exception and exit. This prevents your Python script from hanging indefinitely while waiting for a MOOSE simulation that is likely not running.

For most cases, the default values for these arguments will suffice.

### Subprocess Runners

`SubprocessPortRunner` and `SubprocessSocketRunner` are the subprocess runners. They enable you to execute a MOOSE simulation and write your coupling logic within the same Python script. They take the same optional arguments as all runners as described above, in addition to:

- `command` (required): A list of strings that are the command to use to start the MOOSE process, for example, `['/path/to/moose-opt', '-i', 'foo.i]`.
- `moose_control_name` (required): The name of the [WebServerControl.md] object in your input file. This is needed as the subprocess runner will set an additional command line argument that tells the MOOSE simulation where to listen (either via a port or a socket).
- `directory` (optional): The directory to run the MOOSE process in. Defaults to the current directory.

## Interacting with MOOSE

The `MooseControl` class contains the methods for interacting with the running process.

### General methods

The following methods are the general methods used to interact with the process:

!table
| Method | Description |
| - | - |
| `get_waiting_flag()` | Get the `EXECUTE_ON` flag the [WebServerControl.md] is waiting on, if any |
| `is_waiting()` | Whether or not the control is currently waiting for input |
| `kill()` | Kill the MOOSE process; `set_terminate()` is preferred |
| `set_continue()` | Signals to the control to continue solving |
| `set_terminate()` | Signals to the control to finish solving |
| `wait()` | Wait for the control to be waiting |

### Reading from the simulation id=read_methods

The following methods are used to read data from the running simulation:

!table
| Method | Description |
| - | - |
| `get_postprocessor()` | Get a [`Postprocessor`](Postprocessors/index.md) value by name |
| `get_reporter()` | Get a [`Reporter`](Reporters/index.md) value by name |
| `get_time()` | Get the current simulation time |
| `get_dt()` | Get the current simulation timestep |

They can only be called when the [WebServerControl.md] is waiting (i.e., when `is_waiting()` is `True` or `get_waiting_flag()` is not `None`).

### Setting simulation values id=set_methods

As the [WebServerControl.md] is a [Controls/index.md](Control.md) object, the most common way to change the state of a running simulation is by changing controllable parameters. The following methods are used to change controllable parameters:

!table
| Method | Parameter type |
| - | - |
| `set_bool()` | `bool` |
| `set_int()` | `int` |
| `set_real()` | `Real` |
| `set_string()` | `std::string` |
| `set_vector_int()` | `std::vector<int>` |
| `set_vector_real()` | `std::vector<Real>` |
| `set_vector_string()` | `std::vector<std::string>` |
| `set_realeigenmatrix()` | `RealEigenMatrix` |

It is a fairly simple task to extend these controllable parameter types if needed.

## Basic usage

First, you must create a runner object that defines how the `MooseControl` connects to the running process. For the sake of simplicity, we will use the `SubprocessPortRunner` that will also spawn and manage the MOOSE process for us.

Assume that you have an input file with a [WebServerControl.md] named `web_server`. Create a `SubprocessPortRunner` and initialize the `MooseControl` with this runner as follows:

```language=python
from moosecontrol.runners import SubprocessPortRunner

# Define the runner that will spawn MOOSE over a port
# with the input file "input.i"
moose_command = [
    'mpiexec',
    '-n',
    '2',
    '/path/to/app-opt',
    'i',
    'input.i'
]
runner = SubprocessPortRunner(
    moose_command, # The command to run
    'web_server'   # The name of the WebServerControl in input
)

# Create the MooseControl with our runner
control = MooseControl(runner)
```

You should know which execution flags the webserver will be listening on, and how many times it will be listening.

In the example case that follows, assume that an input exists with:

- The [WebServerControl.md] that executes on `TIMESTEP_BEGIN` and `TIMESTEP_END`
- A postprocessor named `cool_postprocessor`; what it computes is not important
- A boundary condition called `left` which has a controllable `Real` parameter of type `Real`

As the simulation begins (on `TIMESTEP_BEGIN`), we will get the value of the postprocessor in Python. As the simulation ends (on `TIMESTEP_END`), we will modify the boundary condition value in `BCs/left/value`, setting it to a value scaled by the postprocessor value we received at the beginning of the timestep. This is done as follows:

```language=python
# The number of timesteps
num_steps = 10

# Use the MooseControl as a context manager, which automatically
# initializes and finalizes the moose process upon enter and exit.
with control:
    # Loop through the number of expected timesteps
    for t in range(num_steps):
        # Wait for moose to be on TIMESTEP_BEGIN
        control.wait('TIMESTEP_BEGIN')

        # Get a postprocessor value
        pp_value = control.get_postprocessor('cool_postprocessor')

        # Tell moose to continue on TIMESTEP_BEGIN
        control.set_continue()

        # Wait for moose to be done with a solve at TIMESTEP_END
        control.wait('TIMESTEP_END')

        # Set a controllable boundary condition value based on
        # the postprocesor value and the time. This is arbitrary
        # and isn't really physical, but describes usage
        control.set_controllable_real('BCs/left/value', pp_value * t)

        # Tell moose to continue on TIMESTEP_END
        control.set_continue()
```
