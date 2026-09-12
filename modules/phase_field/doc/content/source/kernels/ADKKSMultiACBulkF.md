# KKSMultiACBulkF

!syntax description /Kernels/ADKKSMultiACBulkF

## Description

### Residual

For the 3-phase KKS model, if the non-linear variable is $\eta_1$,

\begin{equation}
R = \left(\frac{\partial h_1}{\partial \eta_1} F_1 + \frac{\partial h_2}{\partial \eta_1} F_2 + \frac{\partial h_3}{\partial \eta_1} F_3 + W_1 \frac{\partial  g_1}{\partial  \eta_1} \right)
\end{equation}

where $c_i$ is the phase concentration for phase $i$ and $h_i$ is the interpolation
function for phase $i$ defined in [!cite](Folch05) (referred to as $g_i$ there, but we use $h_i$ to maintain consistency with other interpolation functions in MOOSE). Here $g_i = \eta_i^2 (1-\eta_i)^2$, also for consistency with notation in MOOSE. $W_1$ is the free energy barrier height.

!syntax parameters /Kernels/ADKKSMultiACBulkF

!syntax inputs /Kernels/ADKKSMultiACBulkF

!syntax children /Kernels/ADKKSMultiACBulkF
