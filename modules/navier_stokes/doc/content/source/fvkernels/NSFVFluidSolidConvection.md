# NSFVFluidSolidConvection

!syntax description /FVKernels/NSFVFluidSolidConvection

## Overview

This object computes the rate of heat transfer from fluid to solid phases,
presumably in a porous region. The residual of this object corresponds to

\begin{equation}
\int_{\Omega_C} \alpha \left(T_f - T_s\right) dV
\end{equation}

where $\alpha$ is the heat transfer coefficient, $T_f$ is the fluid temperature,
and $T_s$ is the solid temperature. Note that this object should only ever be
used for the fluid temperature variable (or whatever variable is being used for
the fluid temperature equation). The solid temperature variable should use
[NSFVSolidFluidConvection.md].

!syntax parameters /FVKernels/NSFVFluidSolidConvection

!syntax inputs /FVKernels/NSFVFluidSolidConvection

!syntax children /FVKernels/NSFVFluidSolidConvection
