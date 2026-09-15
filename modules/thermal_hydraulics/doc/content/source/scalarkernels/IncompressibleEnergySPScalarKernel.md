# IncompressibleEnergySPScalarKernel

!syntax description /ScalarKernels/IncompressibleEnergySPScalarKernel

## Overview

This object implements the time-dependent, globally compressible, locally incompressible energy-transport solve along a single 1D segment for a single variable fluid temperature. Given a coupled variable mass flow rate, a coupled variable upstream fluid temperature, a coupled variable downstream fluid temperature, and a coupled variable wall temperature, all given as (coupled [ScalarVariables](/syntax/Variables)).
Takes fluid properties object based on the ['SinglePhaseFluidProperties'](source/fluidproperties/SinglePhaseFluidProperties.md) base class.
Takes functor inputs for flow area, perimeter, and length.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments. An example of using this
kernel for a system is available, following the INSERT EXAMPLE HERE model.

Some consideration should be given to the [!param](/ScalarKernels/IncompressibleEnergySPScalarKernel/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the last accepted value for the system as the beginning parameter. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.

Rather than using [`ParsedODEKernel`](scalarkernels/ParsedODEKernel.md) and [`ODETimeDerivative`](scalarkernels/ODETimeDerivative.md) kernels, the scalar kernels block can be simplified.

If parameters are to be made available to external control objects, there is still a need to define appropriate [`Postprocessors`](/syntax/Postprocessors) to inform the scalar kernels, as these cannot be assumed for the general case. Otherwise (for constant properties) it is fine to set values directly in the scalar kernel definition.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/IncompressibleEnergySPScalarKernel

!syntax inputs /ScalarKernels/IncompressibleEnergySPScalarKernel

!syntax children /ScalarKernels/IncompressibleEnergySPScalarKernel
