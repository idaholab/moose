# PNSFVPGradEpsilon

!syntax description /FVKernels/PNSFVPGradEpsilon

## Overview

This object adds a residual equivalent to

\begin{equation}
\int_{\Omega_C} -p \nabla \epsilon dV
\end{equation}

This object must be included in any simulations where the $\epsilon \nabla p$
term has been integrated by parts as is done by the [PCNSFVKT.md] and
[PCNSFVHLLC.md] objects.

!syntax parameters /FVKernels/PNSFVPGradEpsilon

!syntax inputs /FVKernels/PNSFVPGradEpsilon

!syntax children /FVKernels/PNSFVPGradEpsilon
