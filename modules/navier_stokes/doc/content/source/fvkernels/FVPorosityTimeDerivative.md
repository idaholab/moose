# FVPorosityTimeDerivative

!syntax description /FVKernels/FVPorosityTimeDerivative

## Overview

This object simply multiplies the residual of [FVTimeKernel.md] by the
porosity. In other words this object computes:

\begin{equation}
\int_{\Omega_C} \epsilon\frac{\partial u}{\partial t} dV
\end{equation}

where $u$ corresponds to the `variable` parameter.

!syntax parameters /FVKernels/FVPorosityTimeDerivative

!syntax inputs /FVKernels/FVPorosityTimeDerivative

!syntax children /FVKernels/FVPorosityTimeDerivative
