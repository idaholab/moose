# IncompressibleMomentumSPScalarKernel

!syntax description /ScalarKernels/IncompressibleMomentumSPScalarKernel

## Overview

This object implements the time-dependent, globally compressible, locally incompressible momentum-transport 1D path integration with an arbitrary number of segments for a
single variable fluid mass flow rate. Given a coupled variable characteristic pressure drop and N coupled fluid temperature variables for each segment, given as (coupled [ScalarVariables](/syntax/Variables)).

The form of the residual contribution is as follows with mass flow rate, \dot{u}, as the primary variable
\begin{equation}
  \sum_{i=1}^{N} \frac{L_i}{A_i} \frac{du}{dt} = - \Delta P_c - \sum_{i=1}^{N} \frac{f_i L_i}{D_{h,i}} \frac{u|u|}{2 \rho_c A_i^2} - \sum_{i=1}^{N} K_i \frac{u|u|}{2 \rho_c A_i^2} - \sum_{i=1}^{N} \rho_i g L_i \sin{\alpha_i} + \Delta P_p \,
\end{equation}

Takes fluid properties object based on the ['SinglePhaseFluidProperties'](source/fluidproperties/SinglePhaseFluidProperties.md) base class.
Takes vector-of-functor inputs for flow area, perimeter, length, angle with respect to horizontal, minor/forms loss coefficients, pump pressures, and component surface roughnesses. This allows one unique geometry to be specified per segment.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments.

Some consideration should be given to the [!param](/ScalarKernels/IncompressibleMomentumSPScalarKernel/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the last accepted value for the system as the beginning parameter. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.

Rather than using [`ParsedODEKernel`](scalarkernels/ParsedODEKernel.md) and [`ODETimeDerivative`](scalarkernels/ODETimeDerivative.md) kernels, the scalar kernels block can be simplified.

If parameters are to be made available to external control objects, there is still a need to define appropriate [`Postprocessors`](/syntax/Postprocessors) to inform the scalar kernels, as these cannot be assumed for the general case. Otherwise (for constant properties) it is fine to set values directly in the scalar kernel definition.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/IncompressibleMomentumSPScalarKernel

!syntax inputs /ScalarKernels/IncompressibleMomentumSPScalarKernel

!syntax children /ScalarKernels/IncompressibleMomentumSPScalarKernel
