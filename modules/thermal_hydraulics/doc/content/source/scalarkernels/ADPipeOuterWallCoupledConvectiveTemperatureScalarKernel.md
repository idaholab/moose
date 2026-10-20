# ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel

## Overview

This class implements the steady-state residual of the outer wall conjugate heat transfer equation for the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md), the right-hand side of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_outer_pipe_temperature). The model is geometry-general: it is driven entirely by functor inputs for areas $A_{i,w}$, perimeters $P_{m,w}$ and $P_{i,w}$, and a radial conduction-path thickness $\delta$, rather than a circular inner/outer diameter, so it is not restricted to hollow circular cross-sections. It requires a coupled variable inner-surface wall
temperature $T_{i,w}$ and coupled variables for the upstream $T_{o,u}$ and downstream $T_{o,d}$ outer-surface wall temperatures, and coupled variables for the secondary-side fluid temperature $T_f$ and its upstream/downstream values, all given as coupled [ScalarVariables](syntax/Variables/index.md). It operates on the outer-surface wall temperature, $T_{o,w}$:

!equation
0 = \frac{1}{A_{o,w} \rho_w c_{p,w}} \left[ \frac{k_w P_{m,w} \left( T_{i,w} - T_{o,w} \right)}{\delta} +
\frac{G_{o,u} \left( T_{o,u} - T_{o,w} \right) + G_{o,d} \left( T_{o,d} - T_{o,w} \right)}{L} + q_o^{'} \right]

This kernel implements a secondary side, coupled convective heat transfer term for the per-length heat transfer term:

!equation
q_o^{'} = \frac{h P_{o,w}}{2} \left ( T_{in,f} + T_f - 2 T_{o,w} \right )

where $h$ is the secondary-side heat transfer coefficient, supplied as a
[functor](/Functors/index.md) through the
[!param](/ScalarKernels/ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel/heat_transfer_coefficient)
parameter. [ADDittusBoelterHTCFunctorMaterial.md] provides it from a Dittus-Boelter
correlation. $T_{in,f}$ upwinds the upstream/downstream secondary-side fluid temperature
according to the sign of the secondary-side mass flow rate $\dot{m}$:

!equation
T_{in,f} = \frac{1}{2} \left(1-\frac{|\dot{m}|}{\dot{m}} \right) T_{d,f} + \frac{1}{2} \left(1+\frac{|\dot{m}|}{\dot{m}} \right) T_{u,f}

Note, use of this kernel with transient problems also necessitates the use of a
[ADScalarTimeDerivative.md] (or its non-AD equivalent) with `coefficient = 1` on the same scalar
variable, which contributes the time derivative term, $\frac{dT_{i,w}}{dt}$, so the two kernels together form the complete transient equation.

This kernel derives from [ADPipeWallTemperatureScalarKernelBase.md], which implements the
axial and radial conduction terms shared by every radial node of the wall; this class adds
the convective heat exchange with the secondary-side flowing fluid.

This kernel takes a solid properties object based on the
[ThermalSolidProperties.md] base class. The heat transfer coefficient and all geometry
inputs (the secondary-side flow channel's wetted perimeter, this node's and its
upstream/downstream neighbors' wall cross-sectional areas, the radial interface perimeter
and thickness, this node's control-volume length, and the axial spacings to each neighbor)
are defined as functors, which should allow versatility in accepting a variety of input
arguments. Furthermore, being functors, it is possible for
them to be controlled via [Controls](syntax/Controls/index.md) as supplied
[Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the
[!param](/ScalarKernels/ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel/is_implicit)
parameter. This term allows the user to select whether the solve should be done with the
current or the previous state values of functor properties. This may allow the system to
evolve more slowly which may avoid some issues with respect to divergence of particularly
unstable systems.

As a reminder, the system of variables should be defined with the
[!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel

!syntax inputs /ScalarKernels/ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel

!syntax children /ScalarKernels/ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel
