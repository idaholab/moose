# IncompressibleMomentumSPScalarKernel

## Overview

This class derives from [IncompressibleMomentumSPBase.md] and implements the steady-state residual of the momentum equation for the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md). Taking the right-hand side of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_momentum). It requires a coupled variable characteristic pressure drop and N coupled fluid temperature variables for each segment, given as (coupled [ScalarVariables](syntax/Variables/index.md)).

!equation
0 = F + \frac{1}{\sum_{i=1}^{n} \frac{L_i}{A_i}} \left [ \Delta P_c + \sum_{i=1}^{N} \left ( \frac{f_i L_i}{D_{h,i}} \frac{u|u|}{2 \rho_c A_i^2} + K_i \frac{u|u|}{2 \rho_c A_i^2} + \rho_i g L_i \sin{\alpha_i} - \Delta P_p \right) \right] \,

Note, use of this kernel with transient problems also necessitates the use of a [ODETimeDerivative.md], which includes the time derivative term, $\frac{du}{dt}$, with $u$ being the mass flow rate, which adds the time derivative of the mass flow rate to the residual:

!equation
\frac{d\dot{m}}{dt} = F \,

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/IncompressibleMomentumSPScalarKernel

!syntax inputs /ScalarKernels/IncompressibleMomentumSPScalarKernel

!syntax children /ScalarKernels/IncompressibleMomentumSPScalarKernel
