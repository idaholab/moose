# ADKKSMultiACBulkC

!syntax description /Kernels/KKSMultiACBulkC

## Description

### Residual

For the 3-phase KKS model, if the non-linear variable is $\eta_1$,

\begin{equation}
R = -\frac{\partial F_1}{\partial c_1} \left( \frac{\partial h_1}{\partial \eta_1} c_1
+ \frac{\partial h_2}{\partial \eta_1} c_2 + \frac{\partial h_3}{\partial \eta_1} c_3 \right)
\end{equation}

where $c_i$ is the phase concentration for phase $i$ and $h_i$ is the interpolation
function for phase $i$ defined in [!cite](Folch05) (referred to as $g_i$ there, but we use $h_i$ to maintain consistency with other interpolation
functions in MOOSE). Since in the KKS model, chemical potentials are constrained to be equal at each
position, $\frac{\partial F_1}{\partial c_1} = \frac{\partial F_2}{\partial c_2}
 = \frac{\partial F_3}{\partial c_3}$.

!syntax parameters /Kernels/ADKKSMultiACBulkC

!syntax inputs /Kernels/ADKKSMultiACBulkC

!syntax children /Kernels/ADKKSMultiACBulkC
