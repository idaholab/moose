# ADCoupledPressureIncompressibleMomentumSPScalarKernel

## Overview

This class derives from [ADIncompressibleMomentumSPBase.md] and implements the steady-state residual of the momentum equation for the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md), the right-hand side of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_momentum). It requires a coupled variable characteristic mass flow rate and $N$ coupled fluid temperature variables for each segment, given as coupled [ScalarVariables](syntax/Variables/index.md). It operates on the characteristic pressure drop, $\Delta P_c$:

!equation
0 = \frac{1}{S} \left [ - \Delta P_c + \sum_{i=1}^{N} \left ( \frac{f_i L_i}{D_{h,i}} \frac{\rho_i v_i |v_i|}{2} + K_i \frac{\rho_i v_i |v_i|}{2} + \rho_i g \Delta h_i - \Delta P_{p,i} \right) \right] \,

where $v_i = \dot{m} / (\rho_{ref,i} A_i)$ is the velocity in segment $i$ and $S = \sum_{i=1}^{N} \max\left(\rho_i / \rho_{ref,i}, 10^{-6}\right) L_i / A_i$ is the inertia scale. The density $\rho_i$ is evaluated at the segment temperature and the reference pressure. Unless [!param](/ScalarKernels/ADCoupledPressureIncompressibleMomentumSPScalarKernel/reference_fp) is provided, $\rho_{ref,i} = \rho_i$, which gives $\rho_i v_i |v_i| = \dot{m}|\dot{m}| / (\rho_i A_i^2)$ and $S = \sum_{i=1}^{N} L_i / A_i$. The friction factor $f_i$ is computed from the segment mass flux $\rho_i v_i$ as described in the theory manual.

This kernel is intended for the use case of creating a closed loop flow path with one or more complementary instances of [ADIncompressibleMomentumSPScalarKernel.md] and potentially a [ParsedODEKernel.md] defining the relationship between mass flow rates in the different flow paths. Thereby creating a closed system of equations for mass flow rates and characteristic pressure drop. Please note, due to the intended use case where this kernel and complementary instances of [ADIncompressibleMomentumSPScalarKernel.md] share mass flow rate information and an equal, but opposite pressure gradient, the sign of the pressure gradient term in the residual is opposite that of the [ADIncompressibleMomentumSPScalarKernel.md].
Furthermore, this kernel is steady-state by default. A transient use case also necessitates a [CoupledODETimeDerivative.md], which includes the time derivative of a coupled variable, $\frac{dv}{dt}$, with $v$ being the coupled reference mass flow rate, which adds the time derivative of the mass flow rate to the residual.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/ADCoupledPressureIncompressibleMomentumSPScalarKernel

!syntax inputs /ScalarKernels/ADCoupledPressureIncompressibleMomentumSPScalarKernel

!syntax children /ScalarKernels/ADCoupledPressureIncompressibleMomentumSPScalarKernel
