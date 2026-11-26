# ADIncompressibleMomentumSPBase

## Overview

This object acts as a base class for the [ADIncompressibleMomentumSPScalarKernel.md] and [ADCoupledPressureIncompressibleMomentumSPScalarKernel.md]. It implements the shared residual terms (friction, forms losses, gravity, pump, and the inertia scaling), while the derived classes supply the mass flow rate and pressure drop used in the residual. The Jacobian is computed with automatic differentiation.

This object takes a fluid properties object based on the [SinglePhaseFluidProperties.md] base class.
Either one fluid properties object (applied to all segments) or one per segment may be given. An optional `reference_fp` object, the liquid-phase fluid properties, may be given for problems in which segments may drain or be filled with gas; see the theory manual.
It also takes vector-of-functor inputs for flow area, perimeter, length, height change from inlet to outlet, minor/forms loss coefficients, pump pressures, and component surface roughnesses. This allows one unique geometry to be specified per segment. The number of entries of each, and of the coupled `temperatures`, must equal the number of segments. The gravitational acceleration, `g`, is also a functor.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments. Furthermore, being functors, it is possible for them to be controlled via [Controls](syntax/Controls/index.md) as supplied [Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the `is_implicit` parameter. This term allows the user to select whether the solve
should be done with the current or the previous state values of functor properties. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.
