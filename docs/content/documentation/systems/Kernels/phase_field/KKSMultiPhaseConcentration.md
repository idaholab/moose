# KKSMultiPhaseConcentration
!syntax description /Kernels/KKSMultiPhaseConcentration

### Residual
For a KKS model with $n$ phases, the residual of the phase concentration constraint equation is
$$
\mathcal{R} = \left( h_1 c_1 + h_2 c_2 + h_3 c_3 + ... + h_n c_n - c  \right)
$$
where $c_i$ is the phase concentration for phase $i$, $c$ is the physical solute concentration, and $h_i$ is the interpolation function for phase $i$ defined in \cite{Folch05} (referred to as $g_i$ there, but we use $h_i$ to maintain consistency with other interpolation functions in MOOSE).

### Jacobian

#### On-diagonal
Since the non-linear variable for this kernel is $c_n$, the on-diagonal Jacobian is
$$
J = \phi_j \frac{\partial R}{\partial c_n}
$$

$$
= \phi_j h_n
$$

#### Off-diagonal
For the physical concentration $c$, the off-diagonal Jacobian is
$$
J = \phi_j \frac{\partial R}{\partial c}
$$

$$
= - \phi_j
$$

For phase concentrations $c_i$ other than $c_n$,:
$$
J = \phi_j \frac{\partial \mathcal{R}}{\partial c_i}
$$

$$
= \phi_j h_i
$$

Finally for the order parameters, such as $\eta_1$,
$$
J = \phi_j \frac{\partial R}{\partial \eta_1}
$$

$$
= \phi_j \left( \frac{\partial h_1}{\partial \eta_1} c_1 + \frac{\partial h_2}{\partial \eta_1} c_2 +  \frac{\partial h_3}{\partial \eta_1} c_3 \right)
$$

For the off-diagonal Jacobians we also need to multiply by $L$, the Allen-Cahn mobility.


!syntax parameters /Kernels/KKSMultiPhaseConcentration

!syntax inputs /Kernels/KKSMultiPhaseConcentration

!syntax children /Kernels/KKSMultiPhaseConcentration

\bibliography{phase_field.bib}
