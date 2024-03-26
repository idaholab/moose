# MooseControl

A helper class used to interact with a simulation running with a [WebServerControl.md], which exposes a REST API for changing the simulation.

## Basic usage

First, instantiate a control object with an input file that has a [WebServerControl.md] object named `control_name`:

```language=python
# Setup the control; runs the application and interacts with it
moose_command = 'mpiexec -n 2 /path/to/app-opt -i input.i'
control = MooseControl(moose_command=moose_command, moose_control_name='control_name')

# Initialize the MOOSE process and wait for the webserver to start
control.initialize()
```

You should know which execution flags the webserver will be listening on, and how many times it will be listening. An example loop over an expected number of timesteps and action on `TIMESTEP_BEGIN` and `TIMESTEP_END` is as follows:

```language=python
# The number of timesteps
num_steps = 10

for t in range(num_steps):
    # Wait for moose to be on TIMESTEP_BEGIN
    control.wait('TIMESTEP_BEGIN')

    # Get a postprocessor value
    pp_value = control.getPostprocesor('postprocesor_name')

    # Tell moose to continue on TIMESTEP_BEGIN
    control.setContinue()

    # Wait for moose to be done with a solve at TIMESTEP_END
    control.wait('TIMESTEP_END')

    # Set a controllable boundary condition value based on
    # the postprocesor value and the time. This is arbitrary
    # and isn't really physical, but describes usage
    control.setControllableReal('BCs/left/value', pp_value * t)

    # Tell moose to continue on TIMESTEP_END
    control.setContinue()

# Wait for MOOSE to finish up
control.finalize()
```

## Class methods

!pysyntax class name=MooseControl.MooseControl heading-level=3 show-internal=False show-private=False show-protected=False
