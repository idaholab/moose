# PipeOuterWallCoupledConvectiveTemperatureScalarKernel

## Overview

This class implements the steady-state residual of the outer wall conjugate heat transfer equation for the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md), the right-hand side of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_outer_pipe_temperature). The model is geometry-general: it is driven entirely by functor inputs for areas $A_{i,w}$, perimeters $P_{m,w}$ and $P_{i,w}$, and a radial conduction-path thickness $\delta$, rather than a circular inner/outer diameter, so it is not restricted to hollow circular cross-sections. It requires a coupled variable inner-surface wall
temperature $T_{i,w}$ and coupled variables for the upstream $T_{o,u}$ and downstream $T_{o,d}$ outer-surface wall temperatures, and coupled variables for the secondary-side fluid temperature $T_f$ and its upstream/downstream values, all given as coupled [ScalarVariables](syntax/Variables/index.md). It operates on the outer-surface wall temperature, $T_{o,w}$:

!equation
0 = \frac{1}{A_{o,w} \rho_w c_{p,w}} \left[ frac{k_w P_{m,w} \left( T_{i,w} - T_{o,w} \right)}{\delta} +
\frac{G_{o,u} \left( T_{o,u} - T_{o,w} \right) + G_{o,d} \left( T_{o,d} - T_{o,w} \right)}{L} + q_o^{'} \right]

This kernel implements a secondary side, coupled convective heat transfer term for the per-length heat transfer term:

!equation
q_o^{'} = \frac{h P_{o,w}}{2} \left ( 2 T_{o,w} - T_{in,f} - T_f \right )

$h$ is computed internally from a Dittus-Boelter correlation using the hydraulic
diameter of the secondary-side flow channel, $D_h = 4 A_f / P_w$, with $A_f$ the flow
channel's cross-sectional area:

!equation
h = 0.023 \, Re^{0.8} Pr^{0.4} \frac{k_f}{D_h}, \qquad
Re = \frac{G D_h}{\mu_f}, \qquad G = \frac{|\dot{m}|}{A_f}, \qquad
Pr = \frac{\mu_f c_{p,f}}{k_f}

where $\dot{m}$ is the secondary-side mass flow rate and $\mu_f$, $c_{p,f}$, and $k_f$ are the secondary-side fluid viscosity, specific heat, and thermal conductivity, evaluated at the reference pressure and the mean of $T_f$ and $T_{in,f}$. $T_{in,f}$ upwinds the
upstream/downstream secondary-side fluid temperature according to the sign of $\dot{m}$:

Note, use of this kernel with transient problems also necessitates the use of a
[ODETimeDerivative.md] (or its AD equivalent) with `coefficient = 1` on the same scalar
variable, which contributes the time derivative term, $\frac{dT_{i,w}}{dt}$, so the two kernels together form the complete transient equation.

This kernel derives from [PipeWallTemperatureScalarKernelBase.md], which implements the
axial and radial conduction terms shared by every radial node of the wall; this class adds
the convective heat exchange with the secondary-side flowing fluid.

This kernel takes a solid properties object based on the
[ThermalSolidProperties.md] base class and a fluid properties object based on the
[SinglePhaseFluidProperties.md] base class, representing the secondary-side fluid. All
geometry inputs (the secondary-side flow channel's area and wetted perimeter, this
node's and its upstream/downstream neighbors' wall cross-sectional areas, the radial
interface perimeter and thickness, this node's control-volume length, and the axial
spacings to each neighbor) are defined as functors, which should allow versatility in
accepting a variety of input arguments. Furthermore, being functors, it is possible for
them to be controlled via [Controls](syntax/Controls/index.md) as supplied
[Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the
[!param](/ScalarKernels/PipeOuterWallCoupledConvectiveTemperatureScalarKernel/is_implicit)
parameter. This term allows the user to select whether the solve should be done with the
current or the previous state values of functor properties. This may allow the system to
evolve more slowly which may avoid some issues with respect to divergence of particularly
unstable systems.

As a reminder, the system of variables should be defined with the
[!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/PipeOuterWallCoupledConvectiveTemperatureScalarKernel

!syntax inputs /ScalarKernels/PipeOuterWallCoupledConvectiveTemperatureScalarKernel

!syntax children /ScalarKernels/PipeOuterWallCoupledConvectiveTemperatureScalarKernel
