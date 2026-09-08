# CoupledPressureIncompressibleMomentumSPScalarKernel

!syntax description /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel

## Overview

This object implements the time-dependent, globally compressible, locally incompressible momentum-transport 1D path integration with an arbitrary number of segments for a
single variable characteristic pressure drop. Given a coupled variable characteristic mass flow rate and N coupled fluid temperature variables for each segment, given as (coupled [ScalarVariables](/syntax/Variables))
This kernel is intended for the use case of creating a closed loop flow path with one or more complementary instances of [`IncompressibleMomentumSPScalarKernel`](scalarkernels/IncompressibleMomentumSPScalarKernel.md) and a [`ParsedODEKernel`](scalarkernels/ParsedODEKernel.md) defining the relationship between mass flow rates in the different flow paths, as shown in the INSERT EXAMPLE HERE model.
Please note, due to the intended use, the pressure gradient term in the residual is opposite that of the [`FlowPathMomentumSCSPScalarKernel`](scalarkernels/FlowPathMomentumSCSPScalarKernel.md).
Furthermore, use of this kernel also necessitates the use of a [`CoupledODETimeDerivative`](scalarkernels/CoupledODETimeDerivative.md) with v being the coupled reference mass flow rate. This may be a temporary requirement if an ADCoupledODETimeDerivative (or similar) object becomes available as a base class for this kernel.
Takes fluid properties object based on the ['SinglePhaseFluidProperties'](source/fluidproperties/SinglePhaseFluidProperties.md) base class.
Takes vector-of-functor inputs for flow area, perimeter, length, angle with respect to horizontal, minor/forms loss coefficients, pump pressures, and component surface roughnesses. This allows one unique geometry to be specified per segment.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments. An example of using this
kernel for a system is available, following the INSERT EXAMPLE HERE model.

Some consideration should be given to the [!param](/ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the last accepted value for the system as the beginning parameter. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.

Rather than using [`ParsedODEKernel`](scalarkernels/ParsedODEKernel.md), the scalar kernels block can be simplified.

If parameters are to be made available to external control objects, there is still a need to define appropriate [`Postprocessors`](/syntax/Postprocessors) to inform the scalar kernels, as these cannot be assumed for the general case. Otherwise (for constant properties) it is fine to set values directly in the scalar kernel definition.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel

!syntax inputs /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel

!syntax children /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel
