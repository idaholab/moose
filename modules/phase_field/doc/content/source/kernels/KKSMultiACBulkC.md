# KKSMultiACBulkC

!syntax description /Kernels/KKSMultiACBulkC

### Residual

For the 3-phase KKS model, if the non-linear variable is $\eta_1$,

\begin{equation}
R = -\frac{\partial F_1}{\partial c_1} \left( \frac{\partial h_1}{\partial \eta_1} c_1 + \frac{\partial h_2}{\partial \eta_1} c_2 + \frac{\partial h_3}{\partial \eta_1} c_3 \right)
\end{equation}

where $c_i$ is the phase concentration for phase $i$ and $h_i$ is the interpolation
function for phase $i$ defined in [!cite](Folch05) (referred to as $g_i$ there, but we use $h_i$ to maintain consistency with other interpolation functions in MOOSE). Since in the KKS model, chemical potentials are constrained to be equal at each position, $\frac{\partial F_1}{\partial c_1} = \frac{\partial F_2}{\partial c_2} = \frac{\partial F_3}{\partial c_3}$.

### Jacobian

#### On-diagonal

If the non-linear variable is $\eta_1$, the on-diagonal Jacobian is

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial \eta_1} \\
&=& -\phi_j \frac{\partial F_1}{\partial c_1} \left( \frac{\partial ^2 h_1}{\partial \eta_1^2} c_1 + \frac{\partial ^2 h_2}{\partial \eta_1^2} c_2 + \frac{\partial ^2 h_3}{\partial \eta_1^2} c_3 \right)
\end{aligned}
\end{equation}

#### Off-diagonal: Phase Concentrations

Since $\frac{\partial F_1}{\partial c_1}$ appears in the residual, the off-diagonal Jacobian for $c_1$ is a little more complicated than for $c_2$ or $c_3$.
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial c_1} \\
&=& -\phi_j \left( \frac{\partial ^2 F_1}{\partial c_1^2} \left[ \frac{\partial  h_1}{\partial \eta_1 } c_1 + \frac{\partial  h_2}{\partial \eta_1} c_2 + \frac{\partial  h_3}{\partial \eta_1} c_3 \right] + \frac{\partial F_1}{\partial c_1} \frac{\partial h_1}{\partial \eta_1} \right)
\end{aligned}
\end{equation}
For $c_2$,
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial c_2} \\
&=& -\phi_j \frac{\partial F_1}{\partial  c_1} \frac{\partial  h_2}{\partial  \eta_1}
\end{aligned}
\end{equation}
and similarly for $c_3$. $c_1$ then $c_2$, and $c_3$ are handled first in the code. For the off-diagonal Jacobians we also need to multiply by $L$, the Allen-Cahn mobility.

#### Off-diagonal: Other Coupled Variables

If the non-linear variable is $\eta_1$, the off-diagonal Jacobian for $\eta_2$ is
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial \eta_2} \\
&=& -\phi_j \frac{\partial F_1}{\partial c_1} \left( \frac{\partial ^2 h_1}{\partial \eta_1 \partial \eta_2} c_1 + \frac{\partial ^2 h_2}{\partial \eta_1 \partial \eta_2} c_2 + \frac{\partial ^2 h_3}{\partial \eta_1 \partial \eta_3} c_3 \right)
\end{aligned}
\end{equation}
and similar for $\eta_3$.

For any other coupled variables, for example temperature $T$
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial T} \\
&=& -\phi_j \frac{\partial ^2 F_1}{\partial c_1 \partial T} \left( \frac{\partial  h_1}{\partial \eta_1 } c_1 + \frac{\partial  h_2}{\partial \eta_1} c_2 + \frac{\partial  h_3}{\partial \eta_1} c_3 \right)
\end{aligned}
\end{equation}

What's implemented in the code for the off-diagonal Jacobian for $\eta_2$,$\eta_3$ and any other coupled variables such as $T$ is the generalization for the above two equations for non-linear variable $v$:
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial v} \\
&=& -\phi_j \left[ \frac{\partial^2 F_1}{\partial c_1 \partial v} \left( \frac{\partial h_1}{\partial \eta_1 } c_1 + \frac{\partial h_2}{\partial \eta_1} c_2 + \frac{\partial h_3}{\partial \eta_1} c_3 \right) +  \frac{\partial F_1}{\partial c_1} \left( \frac{\partial ^2 h_1}{\partial \eta_1 \partial v} c_1 + \frac{\partial ^2 h_2}{\partial \eta_1 \partial v} c_2 + \frac{\partial ^2 h_3}{\partial \eta_1 \partial v} c_3 \right) \right]
\end{aligned}
\end{equation}
(This handles everything except for $c_1$, $c_2$, $c_3$, which are handled separately first). For the off-diagonal Jacobians we also need to multiply by $L$, the Allen-Cahn mobility.

!syntax parameters /Kernels/KKSMultiACBulkC

!syntax inputs /Kernels/KKSMultiACBulkC

!syntax children /Kernels/KKSMultiACBulkC
