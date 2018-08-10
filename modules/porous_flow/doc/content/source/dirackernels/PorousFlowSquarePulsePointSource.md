# PorousFlowSquarePulsePointSource

`PorousFlowSquarePulsePointSource`
implements a constant mass point source that adds (removes) fluid at a constant
mass flux rate for times between the specified start and end times. If
no start and end times are specified, the source (sink) starts at the
start of the simulation and continues to act indefinitely. 
For instance:

!listing modules/porous_flow/test/tests/dirackernels/squarepulse1.i block=DiracKernels

!syntax parameters /DiracKernels/PorousFlowSquarePulsePointSource

!syntax inputs /DiracKernels/PorousFlowSquarePulsePointSource

!syntax children /DiracKernels/PorousFlowSquarePulsePointSource
