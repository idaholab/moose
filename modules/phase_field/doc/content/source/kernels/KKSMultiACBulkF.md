# KKSMultiACBulkF

!syntax description /Kernels/KKSMultiACBulkF

### Residual

For the 3-phase KKS model, if the non-linear variable is $\eta_1$,

\begin{equation}
R = \left(\frac{\partial h_1}{\partial \eta_1} F_1 + \frac{\partial h_2}{\partial \eta_1} F_2 + \frac{\partial h_3}{\partial \eta_1} F_3 + W_1 \frac{\partial  g_1}{\partial  \eta_1} \right)
\end{equation}

where $c_i$ is the phase concentration for phase $i$ and $h_i$ is the interpolation
function for phase $i$ defined in [!cite](Folch05) (referred to as $g_i$ there, but we use $h_i$ to maintain consistency with other interpolation functions in MOOSE). Here $g_i = \eta_i^2 (1-\eta_i)^2$, also for consistency with notation in MOOSE. $W_1$ is the free energy barrier height.

### Jacobian

#### On-diagonal

If the non-linear variable is $\eta_1$, the on-diagonal Jacobian is

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial \eta_1} \\
&=& \phi_j \left( \frac{\partial ^2 h_1}{\partial  \eta_1^2} F_1 + \frac{\partial ^2 h_2}{\partial  \eta_1^2} F_2 + \frac{\partial ^2 h_3}{\partial  \eta_1^2} F_3 + W_1 \frac{\partial ^2 g}{\partial \eta_1^2} \right)
\end{aligned}
\end{equation}

#### Off-diagonal

Off-diagonal Jacobian for $\eta_2$ (similar for $\eta_3$):

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial \eta_2} \\
&=& \phi_j \left( \frac{\partial ^2 h_1}{\partial  \eta_1 \partial  \eta_2} F_1 + \frac{\partial ^2 h_2}{\partial  \eta_1 \partial  \eta_2} F_2 + \frac{\partial ^2 h_3}{\partial  \eta_1 \partial  \eta_2} F_3 \right)
\end{aligned}
\end{equation}

Off-diagonal Jacobian for $c_1$ (similar for $c_2, c_3$):

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial c_1} \\
&=& \phi_j \frac{\partial  h_1}{\partial  \eta_1} \frac{\partial  F_1}{\partial  c_1}
\end{aligned}
\end{equation}

These statements can be generalized for non-linear variable $v$ as:

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial R}{\partial v} \\
&=& \left( \frac{\partial ^2 h_1}{\partial  \eta_1 \partial  v} F_1 + \frac{\partial ^2 h_2}{\partial  \eta_1 \partial  v} F_2 + \frac{\partial ^2 h_3}{\partial  \eta_1 \partial  v} F_3 + \frac{\partial  h_1}{\partial  \eta_1} \frac{\partial  F_1}{\partial  v} + \frac{\partial  h_2}{\partial  \eta_1} \frac{\partial  F_2}{\partial  v} + \frac{\partial  h_3}{\partial  \eta_1} \frac{\partial  F_3}{\partial  v}\right)
\end{aligned}
\end{equation}

For the off-diagonal Jacobians we also need to multiply by $L$, the Allen-Cahn mobility.

!syntax parameters /Kernels/KKSMultiACBulkF

!syntax inputs /Kernels/KKSMultiACBulkF

!syntax children /Kernels/KKSMultiACBulkF
