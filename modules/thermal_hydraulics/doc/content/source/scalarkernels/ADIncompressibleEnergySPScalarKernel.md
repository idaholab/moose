# ADIncompressibleEnergySPScalarKernel

## Overview

This class implements the steady-state residual of the energy equation for the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md), the right-hand side of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_energy). It requires a coupled variable mass flow rate, a coupled variable upstream fluid temperature, a coupled variable downstream fluid temperature, and a coupled variable wall temperature, all given as coupled [ScalarVariables](syntax/Variables/index.md). It operates on the segment temperature, $T$:

!equation
0 = \frac{\dot{m}}{2 L A \rho_{ref}} \left(1 - s\right) T_d - \frac{\dot{m}}{2 L A \rho_{ref}} \left(1 + s\right) T_u + \frac{\dot{m} s}{L A \rho_{ref}} T - \frac{h P_w}{A \rho c_p} \left( T_w - T_m \right) \,

where $s = \dot{m} / \sqrt{\dot{m}^2 + 10^{-8}}$ is a smoothed sign of the mass flow rate (equal to $|\dot{m}| / \dot{m}$ away from zero flow, with the constant $10^{-8}\ \mathrm{kg^2/s^2}$ regularizing it at $\dot{m} = 0$), $T_m$ is the bulk temperature used for wall heat transfer, and $\rho$ and $c_p$ are evaluated at $T_m$ and the reference pressure. The density $\rho_{ref}$ is the density of the reference liquid at $T_m$ if [!param](/ScalarKernels/ADIncompressibleEnergySPScalarKernel/reference_fp) is given, and $\rho$ otherwise. The bulk temperature is selected with `heat_transfer_bulk_temperature`:

- `segment_average` (default): $T_m = \left( T + T_{in} \right) / 2$, with $T_{in} = \frac{1}{2}(1-s) T_d + \frac{1}{2}(1+s) T_u$ the upwinded neighbor temperature. This recovers the wall term of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_energy).
- `node`: $T_m = T$. This is monotone for any number of transfer units and any flow rate, whereas `segment_average` is not monotone for more than two transfer units and, at zero flow, cannot damp an odd/even mode of the node temperatures. It must match the setting of the coupled wall temperature kernel so that the wall and fluid exchange the same heat.

Note, use of this kernel with transient problems also necessitates the use of a [ODETimeDerivative.md] or [ADScalarTimeDerivative.md], which adds the time derivative of the segment temperature, $\frac{dT}{dt}$, to the residual.

This kernel takes a fluid properties object based on the [SinglePhaseFluidProperties.md] base class.
It also takes functor inputs for flow area, perimeter, length, and heat transfer coefficient.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments. Furthermore, being functors, it is possible for them to be controlled via [Controls](syntax/Controls/index.md) as supplied [Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the [!param](/ScalarKernels/ADIncompressibleEnergySPScalarKernel/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the previous state values of functor properties. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/ADIncompressibleEnergySPScalarKernel

!syntax inputs /ScalarKernels/ADIncompressibleEnergySPScalarKernel

!syntax children /ScalarKernels/ADIncompressibleEnergySPScalarKernel
