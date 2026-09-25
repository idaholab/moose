# IncompressibleMomentumSPBase

## Overview

This object acts as a base class for the [IncompressibleMomentumSPScalarKernel.md] and [CoupledPressureIncompressibleMomentumSPScalarKernel.md]. It contains the shared input parameters but does not implement any residual terms or Jacobian.

This object takes a fluid properties object based on the [SinglePhaseFluidProperties.md] base class.
It also takes vector-of-functor inputs for flow area, perimeter, length, angle with respect to horizontal, minor/forms loss coefficients, pump pressures, and component surface roughnesses. This allows one unique geometry to be specified per segment.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments. Furthermore, being functors, it is possible for them to be controlled via [Controls](syntax/Controls/index.md) as supplied [Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the [!param](/ScalarKernels/IncompressibleMomentumSPBase/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the previous state values of functor properties. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.
