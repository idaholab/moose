# CoupledPressureIncompressibleMomentumSPScalarKernel

!syntax description /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel

## Overview

This object implements the time-dependent, globally compressible, locally incompressible momentum-transport 1D path integration with an arbitrary number of segments for a
single variable characteristic pressure drop. Given a coupled variable characteristic mass flow rate and N coupled fluid temperature variables for each segment, given as (coupled [ScalarVariables](syntax/Variables/index.md))

The form of the residual contribution is as follows with system characteristic pressure drop, \dot{u}, as the primary variable
\begin{equation}
  0 = - \sum_{i=1}^{N} \frac{A_i}{L_i} u - \sum_{i=1}^{N} \frac{f_i}{D_{h,i}} \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i} - \sum_{i=1}^{N} K_i \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i L_i} - \sum_{i=1}^{N} \rho_i g A_i \sin{\alpha_i} + \sum_{i=1}^{N} \frac{A_i}{L_i} \Delta P_p \,
\end{equation}


This kernel is intended for the use case of creating a closed loop flow path with one or more complementary instances of [IncompressibleMomentumSPScalarKernel.md] and a [ParsedODEKernel.md] defining the relationship between mass flow rates in the different flow paths.
Please note, due to the intended use, the pressure gradient term in the residual is opposite that of the [IncompressibleMomentumSPScalarKernel.md].
Furthermore, use of this kernel also necessitates the use of a [CoupledODETimeDerivative.md] with v being the coupled reference mass flow rate, which adds the time derivative of the mass flow rate to the residual:
\begin{equation}
  \frac{d\dot{m}}{dt} = - \sum_{i=1}^{N} \frac{A_i}{L_i} u - \sum_{i=1}^{N} \frac{f_i}{D_{h,i}} \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i} - \sum_{i=1}^{N} K_i \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i L_i} - \sum_{i=1}^{N} \rho_i g A_i \sin{\alpha_i} + \sum_{i=1}^{N} \frac{A_i}{L_i} \Delta P_p \,
\end{equation}
This may be a temporary requirement if an ADCoupledODETimeDerivative (or similar) object becomes available as a base class for this kernel.

Takes fluid properties object based on the [SinglePhaseFluidProperties.md] base class.
Takes vector-of-functor inputs for flow area, perimeter, length, angle with respect to horizontal, minor/forms loss coefficients, pump pressures, and component surface roughnesses. This allows one unique geometry to be specified per segment.
All parameters are defined as functors,
which should allow versatility in accepting a variety of input arguments.

Some consideration should be given to the [!param](/ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel/is_implicit) parameter. This term allows the user to select whether the solve
should be done with the current or the previous state values of functor properties. This may allow the system to evolve more slowly which may avoid some issues with respect to divergence of particularly unstable systems.

Rather than using [ParsedODEKernel.md], the scalar kernels block can be simplified.

If parameters are to be made available to external control objects, there is still a need to define appropriate [Postprocessors](syntax/Postprocessors/index.md) to inform the scalar kernels, as these cannot be assumed for the general case. Otherwise (for constant properties) it is fine to set values directly in the scalar kernel definition.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel

!syntax inputs /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel

!syntax children /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel
