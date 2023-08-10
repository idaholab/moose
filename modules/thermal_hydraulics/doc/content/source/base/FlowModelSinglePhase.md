# FlowModelSinglePhase

The `FlowModelSinglePhase` creates the following for [FlowChannel1Phase.md] components using the 3-equation model:

- variables
- initial conditions
- kernels and DGKernels
- auxiliary kernels to compute density, velocity etc
- material properties needed for the kernels
- flux objects for HLLC

The full list of the objects created is listed in the [FlowChannel1Phase documentation](FlowChannel1Phase.md).

!alert note
The `FlowModelSinglePhase` is created by [FlowChannel1Phase.md] components. It is not specified directly
in the input file.
