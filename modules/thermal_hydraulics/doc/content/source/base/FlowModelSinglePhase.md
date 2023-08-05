# FlowModelSinglePhase

The `FlowModelSinglePhase` creates for [FlowChannel1Phase.md] components using the three equation model:

- variables
- initial conditions
- kernels and DGKernels for RDG
- auxiliary kernels to compute density, velocity etc
- material properties needed for the kernels
- flux objects for HLLC

The full list of the objects created is listed in the [FlowChannel1Phase documentation](FlowChannel1Phase.md).

!alert note
The `FlowModelSinglePhase` is created by [FlowChannel1Phase.md] components. It is not specified directly
in the input file.
