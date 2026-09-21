# IncompressibleEnergySPScalarKernel

!syntax description /ScalarKernels/IncompressibleEnergySPScalarKernel

## Overview

This object implements the time-dependent, globally compressible, locally incompressible, single-phase, energy-transport solve along a single 1D segment for a single variable fluid temperature. It requires a coupled variable mass flow rate, a coupled variable upstream fluid temperature, a coupled variable downstream fluid temperature, and a coupled variable wall temperature, all given as (coupled [ScalarVariables](syntax/Variables/index.md)).

!equation
\begin{equation}
  A \rho c_p \frac{du}{dt} + \frac{\dot{m}}{2 L} \left(1 - \frac{|\dot{m}|}{\dot{m}}\right) c_p T_d - \frac{\dot{m}}{2 L} \left(1 + \frac{|\dot{m}|}{\dot{m}}\right) c_p T_u + \frac{|\dot{m}| c_p}{L} u = \frac{h P_w}{2} \left( 2 T_w - u - \frac{1}{2} \left( 1 - \frac{|\dot{m}|}{\dot{m}} \right) T_d - \frac{1}{2} \left( 1 + \frac{|\dot{m}|}{\dot{m}} \right) T_u \right) \,
\end{equation}

This kernel takes a fluid properties object based on the [SinglePhaseFluidProperties.md] base class.
It also takes functor inputs for flow area, perimeter, and length.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments.

Some consideration should be given to the [!param](/ScalarKernels/IncompressibleEnergySPScalarKernel/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the previous state values of functor properties. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.

Rather than using [ParsedODEKernel.md] and [ODETimeDerivative.md] kernels, the scalar kernels block can be simplified.

If parameters are to be made available to external control objects, there is still a need to define appropriate [Postprocessors](syntax/Postprocessors/index.md) to inform the scalar kernels, as these cannot be assumed for the general case. Otherwise (for constant properties) it is fine to set values directly in the scalar kernel definition.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/IncompressibleEnergySPScalarKernel

!syntax inputs /ScalarKernels/IncompressibleEnergySPScalarKernel

!syntax children /ScalarKernels/IncompressibleEnergySPScalarKernel
