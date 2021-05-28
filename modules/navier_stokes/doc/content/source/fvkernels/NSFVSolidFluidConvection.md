# NSFVSolidFluidConvection

!syntax description /FVKernels/NSFVSolidFluidConvection

## Overview

This object computes a residual that is the negative of
[NSFVFluidSolidConvection.md]. In other words, its residual is given by

\begin{equation}
\int_{\Omega_C} \alpha \left(T_s - T_f\right) dV
\end{equation}

where $\alpha$ is the heat transfer coefficient, $T_f$ is the fluid temperature,
and $T_s$ is the solid temperature. Note that this object should only ever be
used for the solid temperature variable. The fluid temperature variable should use
[NSFVFluidSolidConvection.md].

!syntax parameters /FVKernels/NSFVSolidFluidConvection

!syntax inputs /FVKernels/NSFVSolidFluidConvection

!syntax children /FVKernels/NSFVSolidFluidConvection
