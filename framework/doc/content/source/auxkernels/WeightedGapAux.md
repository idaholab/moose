# WeightedGapAux

!syntax description /AuxKernels/WeightedGapAux

## Overview

This object computes a discrete weighted gap $\tilde{g}_i$ in a mortar context according to

\begin{equation}
\tilde{g}_i = \frac{\int_{\Gamma_c^{(1)}} \psi_i g_h dA}{\int_{\Gamma_c^{(1)}} dA}
\end{equation}

where $\Gamma_c^{(1)}$ denotes the secondary contact interface, $\psi_i$ is the $i$th
shape function associated with the auxiliary variable this aux kernel is
computing the value for and $g_h$ is the discretized version of the gap
function, computed in this aux kernel as the difference in quadrature point
location between primary and secondary faces times the normal vector.

!syntax parameters /AuxKernels/WeightedGapAux

!syntax inputs /AuxKernels/WeightedGapAux

!syntax children /AuxKernels/WeightedGapAux
