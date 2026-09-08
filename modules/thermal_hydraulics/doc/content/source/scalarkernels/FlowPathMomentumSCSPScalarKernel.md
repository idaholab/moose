# FlowPathMomentumSCSPScalarKernel

!syntax description /ScalarKernels/FlowPathMomentumSCSPScalarKernel

## Overview

This object implements the time-dependent, globally compressible, locally incompressible momentum-transport 1D path integration with an arbitrary number of segments for a
single variable fluid mass flow rate. Given a coupled variable characteristic pressure drop and N coupled fluid temperature variables for each segment, given as (coupled [ScalarVariables](/syntax/Variables)).
Takes fluid properties object based on the ['SinglePhaseFluidProperties'](source/fluidproperties/SinglePhaseFluidProperties.md) base class.
Takes vector-of-functor inputs for flow area, perimeter, length, angle with respect to horizontal, minor/forms loss coefficients, pump pressures, and component surface roughnesses. This allows one unique geometry to be specified per segment.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments. An example of using this
kernel for a system is available, following the INSERT EXAMPLE HERE model.

Some consideration should be given to the [!param](/ScalarKernels/FlowPathMomentumSCSPScalarKernel/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the last accepted value for the system as the beginning parameter. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.

Rather than using [`ParsedODEKernel`](scalarkernels/ParsedODEKernel.md) and [`ODETimeDerivative`](scalarkernels/ODETimeDerivative.md) kernels, the scalar kernels block can be simplified.

If parameters are to be made available to external control objects, there is still a need to define appropriate [`Postprocessors`](/syntax/Postprocessors) to inform the scalar kernels, as these cannot be assumed for the general case. Otherwise (for constant properties) it is fine to set values directly in the scalar kernel definition.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/FlowPathMomentumSCSPScalarKernel

!syntax inputs /ScalarKernels/FlowPathMomentumSCSPScalarKernel

!syntax children /ScalarKernels/FlowPathMomentumSCSPScalarKernel
