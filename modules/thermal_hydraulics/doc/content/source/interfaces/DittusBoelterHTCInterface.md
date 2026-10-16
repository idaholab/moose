# DittusBoelterHTCInterface

This interface supplies the wall heat transfer coefficient $h$ of a lumped flow segment of
the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md)
from the Dittus-Boelter correlation,
[!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#dittus_boelter).
Objects that need that coefficient inherit from it instead of taking $h$ as an input, which
is how the conjugate heat transfer scalar kernels
[ADPipeInnerWallTemperatureScalarKernel.md] and
[ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel.md] obtain the coefficient of their
respective sides of the wall.

The Reynolds number is formed from the segment's mass flux, $G = \dot{m} / A$, so no density
evaluation is needed. Fluid properties are evaluated at the segment's bulk film temperature,
the mean of the segment fluid temperature $T_f$ and its upwinded inlet value $T_{in}$, taken
from the nominal inlet or outlet neighbor according to the sign of $\dot{m}$.

The correlation needs the flow state of the segment and the geometry of its flow channel.
The flow state is already held by the inheriting object, which supplies it by implementing
the interface's pure virtual methods: the mass flow rate $\dot{m}$, the segment fluid
temperature $T_f$, the fluid temperatures of the nominal inlet and outlet neighbors, and the
wetted perimeter of the flow channel. The interface itself therefore contributes only the
three remaining inputs, as parameters of the inheriting object:

- `fluid_properties`: the [SinglePhaseFluidProperties.md] object of the flowing fluid,
- `reference_pressure`: the system reference pressure at which fluid properties are
  evaluated, and
- `flow_area`: the cross-sectional area $A$ of the flow channel, which together with the
  wetted perimeter gives the hydraulic diameter $D_h$.

`reference_pressure` and `flow_area` are [functions](syntax/Functions/index.md), as are the
geometry inputs of the inheriting kernels, so they may be made time-dependent or controlled
via [Controls](syntax/Controls/index.md).
