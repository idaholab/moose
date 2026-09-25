# IncompressibleEnergySPScalarKernel

## Overview

This object implements non-transient terms for a globally compressible, locally incompressible, single-phase, energy-transport solve along a single 1D segment for a single variable fluid temperature. See the theory manual for more details [theory manual](modules/thermal_hydraulics/theory_manual/index.md). It requires a coupled variable mass flow rate, a coupled variable upstream fluid temperature, a coupled variable downstream fluid temperature, and a coupled variable wall temperature, all given as (coupled [ScalarVariables](syntax/Variables/index.md)).

!equation
0 = F + \frac{\dot{m}}{2 L A \rho} \left(1 - \frac{|\dot{m}|}{\dot{m}}\right) T_d - \frac{\dot{m}}{2 L A \rho} \left(1 + \frac{|\dot{m}|}{\dot{m}}\right) T_u + \frac{|\dot{m}|}{L A \rho} u - \frac{h P_w}{2 A \rho c_p} \left[ 2 T_w - u - \frac{1}{2} \left( 1 - \frac{|\dot{m}|}{\dot{m}} \right) T_d - \frac{1}{2} \left( 1 + \frac{|\dot{m}|}{\dot{m}} \right) T_u \right] \,

Note, use of this kernel also necessitates the use of a [ODETimeDerivative.md], which includes the time derivative term, $\frac{du}{dt}$, with $u$ being the segment temeprature, which adds the time derivative of the segment temperature to the residual:

!equation
\frac{dT}{dt} = F \,

This kernel takes a fluid properties object based on the [SinglePhaseFluidProperties.md] base class.
It also takes functor inputs for flow area, perimeter, and length.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments. Furthermore, being functors, it is possible for them to be controlled via [Controls](syntax/Controls/index.md) as supplied [Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the [!param](/ScalarKernels/IncompressibleEnergySPScalarKernel/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the previous state values of functor properties. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/IncompressibleEnergySPScalarKernel

!syntax inputs /ScalarKernels/IncompressibleEnergySPScalarKernel

!syntax children /ScalarKernels/IncompressibleEnergySPScalarKernel
