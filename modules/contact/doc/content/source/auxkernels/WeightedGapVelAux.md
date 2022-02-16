# WeightedGapVelAux

!syntax description /AuxKernels/WeightedGapVelAux

## Overview

This object computes a discrete weighted gap velocity $\dot{\tilde{g}}_i$ in a mortar context according to

\begin{equation}
\dot{\tilde{g}}_i = \frac{\int_{\Gamma_c^{(1)}} \psi_i \dot{g}_h dA}{\int_{\Gamma_c^{(1)}} dA}
\end{equation}

where $\Gamma_c^{(1)}$ denotes the secondary contact interface, $\psi_i$ is the $i$th
shape function associated with the auxiliary variable this aux kernel is
computing the value for and $\dot{g}_h$ is the discretized version of the gap velocity
function, computed in this aux kernel as the difference in quadrature point spatial velocity
between primary and secondary faces times the normal vector.

!syntax parameters /AuxKernels/WeightedGapVelAux

!syntax inputs /AuxKernels/WeightedGapVelAux

!syntax children /AuxKernels/WeightedGapVelAux
