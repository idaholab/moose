# ADIncompressibleMomentumSPScalarKernel

## Overview

This class derives from [ADIncompressibleMomentumSPBase.md] and implements the steady-state residual of the momentum equation for the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md), the negative of the right-hand side of [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_momentum), normalized by the inertia scale $S$ defined below so that the time derivative term has unit coefficient. It requires a coupled variable characteristic pressure drop and $N$ coupled fluid temperature variables for each segment, given as coupled [ScalarVariables](syntax/Variables/index.md). It operates on the mass flow rate, $\dot{m}$:

!equation
0 = \frac{1}{S} \left [ \Delta P_c + \sum_{i=1}^{N} \left ( \frac{f_i L_i}{D_{h,i}} \frac{\rho_i v_i |v_i|}{2} + K_i \frac{\rho_i v_i |v_i|}{2} + \rho_i g \Delta h_i - \Delta P_{p,i} \right) \right] \,

where $v_i = \dot{m} / (\rho_{ref,i} A_i)$ is the velocity in segment $i$ and $S = \sum_{i=1}^{N} \max\left(\rho_i / \rho_{ref,i}, 10^{-6}\right) L_i / A_i$ is the inertia scale. The density $\rho_i$ is evaluated at the segment temperature and the reference pressure. Unless [!param](/ScalarKernels/ADIncompressibleMomentumSPScalarKernel/reference_fp) is provided, $\rho_{ref,i} = \rho_i$, which gives $\rho_i v_i |v_i| = \dot{m}|\dot{m}| / (\rho_i A_i^2)$ and $S = \sum_{i=1}^{N} L_i / A_i$. The friction factor $f_i$ is computed from the segment mass flux $\rho_i v_i$ as described in the theory manual.

Note, use of this kernel with transient problems also necessitates the use of a [ODETimeDerivative.md] or [ADScalarTimeDerivative.md], which includes the time derivative term, $\frac{du}{dt}$, with $u$ being the mass flow rate, which adds the time derivative of the mass flow rate to the residual.

As a reminder, the system of variables should be defined with the [!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/ADIncompressibleMomentumSPScalarKernel

!syntax inputs /ScalarKernels/ADIncompressibleMomentumSPScalarKernel

!syntax children /ScalarKernels/ADIncompressibleMomentumSPScalarKernel
