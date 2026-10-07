# PipeInnerWallTemperatureScalarKernel

## Overview

This class implements the steady-state residual of the inner wall conjugate heat transfer equation for the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md), the right-hand side of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_inner_pipe_temperature). The model is geometry-general: it is driven entirely by functor inputs for areas $A_{i,w}$, perimeters $P_{m,w}$ and $P_{i,w}$, and a radial conduction-path thickness $\delta$, rather than a circular inner/outer diameter, so it is not restricted to hollow circular cross-sections. It requires a coupled variable mass flow rate $\dot{m}$, a coupled variable outer-surface wall temperature $T_{o,w}$, coupled variables for the upstream $T_{i,u}$ and downstream $T_{i,d}$ inner-surface wall temperatures, and coupled variables for the fluid temperature $T_f$ and its upstream/downstream values, all given as coupled [ScalarVariables](syntax/Variables/index.md). It operates on the inner-surface wall temperature, $T_{i,w}$:

!equation
0 = \frac{1}{A_{i,w} \rho_w c_{p,w}} \left[ \frac{k_w P_{m,w} \left( T_{o,w} - T_{i,w} \right)}{\delta} + \frac{G_{i,u} \left( T_{i,u} - T_{i,w} \right) + G_{i,d} \left( T_{i,d} - T_{i,w} \right)}{L} + h \frac{P_{i,w}}{2} \left( T_f + T_{in} - 2T_{i,w} \right) \right]

Note, use of this kernel with transient problems also necessitates the use of a
[ODETimeDerivative.md] (or its AD equivalent) with `coefficient = 1` on the same scalar
variable, which contributes the time derivative term, $\frac{dT_{i,w}}{dt}$, so the two kernels together form the complete transient equation.

This kernel derives from [PipeWallTemperatureScalarKernelBase.md], which implements the
axial and radial conduction terms shared by every radial node of the wall; this class adds
the convective heat exchange with the primary flowing fluid.

This kernel takes a solid properties object based on the
[ThermalSolidProperties.md] base class and a fluid properties object based on the
[SinglePhaseFluidProperties.md] base class. All geometry inputs (the flow channel's area
and wetted perimeter, this node's and its upstream/downstream neighbors' wall
cross-sectional areas, the radial interface perimeter and thickness, this node's
control-volume length, and the axial spacings to each neighbor) are defined as functors,
which should allow versatility in accepting a variety of input arguments. Furthermore,
being functors, it is possible for them to be controlled via
[Controls](syntax/Controls/index.md) as supplied
[Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the
[!param](/ScalarKernels/PipeInnerWallTemperatureScalarKernel/is_implicit) parameter. This
term allows the user to select whether the solve should be done with the current or the
previous state values of functor properties. This may allow the system to evolve more
slowly which may avoid some issues with respect to divergence of particularly unstable
systems.

As a reminder, the system of variables should be defined with the
[!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/PipeInnerWallTemperatureScalarKernel

!syntax inputs /ScalarKernels/PipeInnerWallTemperatureScalarKernel

!syntax children /ScalarKernels/PipeInnerWallTemperatureScalarKernel
