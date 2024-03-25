# MooseControl

A helper class used to interact with a simulation running with a [WebServerControl.md], which exposes a REST API for changing the simulation.

## Basic usage

First, instantiate a control object with the port that the [WebServerControl.md] is running on and wait for MOOSE to be available:

```language=python
# Setup the control to interact with the server on port 12345
control = MooseControl(12345)

# Waits for the MOOSE webserver to be listening
control.initialWait()
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
control.finalWait()
```

## Class methods

!pysyntax class name=MooseControl.MooseControl heading-level=3 show-internal=False show-private=False show-protected=False
