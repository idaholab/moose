# PCNSFVDensityTimeDerivative

!syntax description /FVKernels/PCNSFVDensityTimeDerivative

## Overview

This object computes the residual

\begin{equation}
\int_{\Omega_C} \epsilon \frac{\partial\left(\rho u\right)}{\partial t} dV
\end{equation}

where $\epsilon$ is the porosity, $\rho$ is the density, and $u$ corresponds to
the variable specified with the `variable` parameter. This object currently only
works if $\rho$ is a variable (e.g. this doesn't work if the only computation of
$\partial \rho/\partial t$ occurs in a material).

!syntax parameters /FVKernels/PCNSFVDensityTimeDerivative

!syntax inputs /FVKernels/PCNSFVDensityTimeDerivative

!syntax children /FVKernels/PCNSFVDensityTimeDerivative
