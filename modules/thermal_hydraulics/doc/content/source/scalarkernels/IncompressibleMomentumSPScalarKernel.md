# IncompressibleMomentumSPScalarKernel

!syntax description /ScalarKernels/IncompressibleMomentumSPScalarKernel

## Overview

This object implements non-transient terms for a globally compressible, locally incompressible, single-phase, momentum-transport 1D path integration with an arbitrary number of segments for a
single variable fluid mass flow rate. See the theory manual for more details [theory manual](modules/thermal_hydraulics/theory_manual/index.md). It requires a coupled variable characteristic pressure drop and N coupled fluid temperature variables for each segment, given as (coupled [ScalarVariables](syntax/Variables/index.md)).

!equation
0 = F + \frac{1}{\sum_{i=1}^{n} \frac{L_i}{A_i}} \left [ \Delta P_c + \sum_{i=1}^{N} \left ( \frac{f_i L_i}{D_{h,i}} \frac{u|u|}{2 \rho_c A_i^2} + K_i \frac{u|u|}{2 \rho_c A_i^2} + \rho_i g L_i \sin{\alpha_i} - \Delta P_p \right) \right] \,

Note, use of this kernel also necessitates the use of a [ODETimeDerivative.md], which includes the time derivative term, $\frac{du}{dt}$, with $u$ being the mass flow rate, which adds the time derivative of the mass flow rate to the residual:

!equation
\frac{d\dot{m}}{dt} = F \,

Rather than using [ParsedODEKernel.md] and [ODETimeDerivative.md] kernels, the scalar kernels block can be simplified.

If parameters are to be made available to external control objects, there is still a need to define appropriate [Postprocessors](syntax/Postprocessors/index.md) to inform the scalar kernels, as these cannot be assumed for the general case. Otherwise (for constant properties) it is fine to set values directly in the scalar kernel definition.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/IncompressibleMomentumSPScalarKernel

!syntax inputs /ScalarKernels/IncompressibleMomentumSPScalarKernel

!syntax children /ScalarKernels/IncompressibleMomentumSPScalarKernel
