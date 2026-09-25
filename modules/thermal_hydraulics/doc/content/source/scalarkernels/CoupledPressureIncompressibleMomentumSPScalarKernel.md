# CoupledPressureIncompressibleMomentumSPScalarKernel

## Overview

This class derives from [IncompressibleMomentumSPBase.md] and implements the steady-state residual of the momentum equation for the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md). Taking the right-hand side of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_momentum). It requires a coupled variable characteristic mass flow rate and N coupled fluid temperature variables for each segment, given as (coupled [ScalarVariables](syntax/Variables/index.md)).

!equation
0 = F + \frac{1}{\sum_{i=1}^{n} \frac{L_i}{A_i}} \left [ - u + \sum_{i=1}^{N} \left ( \frac{f_i L_i}{D_{h,i}} \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i^2} + K_i \frac{\dot{m}|\dot{m}|}{2 \rho_c A_i^2} + \rho_i g L_i \sin{\alpha_i} - \Delta P_p \right) \right] \,

This kernel is intended for the use case of creating a closed loop flow path with one or more complementary instances of [IncompressibleMomentumSPScalarKernel.md] and a [ParsedODEKernel.md] defining the relationship between mass flow rates in the different flow paths.
Please note, due to the intended use, the pressure gradient term in the residual is opposite that of the [IncompressibleMomentumSPScalarKernel.md].
Furthermore, use of this kernel also necessitates the use of a [CoupledODETimeDerivative.md], which includes the time derivative of a coupled variable, $\frac{dv}{dt}$, with $v$ being the coupled reference mass flow rate, which adds the time derivative of the mass flow rate to the residual:

!equation
\frac{d\dot{m}}{dt} = F \,

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel

!syntax inputs /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel

!syntax children /ScalarKernels/CoupledPressureIncompressibleMomentumSPScalarKernel
