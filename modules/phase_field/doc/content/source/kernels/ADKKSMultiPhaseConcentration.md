# ADKKSMultiPhaseConcentration

!syntax description /Kernels/ADKKSMultiPhaseConcentration

## Description

### Residual

For a KKS model with $n$ phases, the residual of the phase concentration
constraint equation is

\begin{equation}
R = \left( h_1 c_1 + h_2 c_2 + h_3 c_3 + \dots + h_n c_n - c  \right)
\end{equation}

where $c_i$ is the phase concentration for phase $i$, $c$ is the physical solute
concentration, and $h_i$ is the interpolation function for phase $i$ defined in
[!cite](Folch05) (referred to as $g_i$ there, but we use $h_i$ to maintain
consistency with other interpolation functions in MOOSE).

!syntax parameters /Kernels/ADKKSMultiPhaseConcentration

!syntax inputs /Kernels/ADKKSMultiPhaseConcentration

!syntax children /Kernels/ADKKSMultiPhaseConcentration
