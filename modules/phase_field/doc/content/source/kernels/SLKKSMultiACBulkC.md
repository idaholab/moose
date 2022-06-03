# SLKKSMultiACBulkC

!syntax description /Kernels/SLKKSMultiACBulkC

Implements the weak form
\begin{equation}
\left( -M\frac1{a_{jk}}\frac{\partial F_j}{\partial c_ijk} \sum_j \frac{\partial h_j}{\partial \eta_j}\sum_k c_{ijk}a_{jk},\psi\right)
\end{equation}

where $c_i$ is the phase concentration for phase $i$ and $h_i$ is the interpolation
function for phase $i$ defined in [!cite](Folch05) (referred to as $g_i$ there, but we use $h_i$ to maintain consistency with other interpolation functions in MOOSE). Since in the KKS model, chemical potentials are constrained to be equal at each position, $\frac{\partial F_1}{\partial c_1} = \frac{\partial F_2}{\partial c_2} = \frac{\partial F_3}{\partial c_3}$.

!syntax parameters /Kernels/SLKKSMultiACBulkC

!syntax inputs /Kernels/SLKKSMultiACBulkC

!syntax children /Kernels/SLKKSMultiACBulkC
