<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# KKSMultiPhaseConcentration
!description /Kernels/KKSMultiPhaseConcentration

### Residual
The residual of the phase concentration constraint equation is
$$
R = \left( h_1 c_1 + h_2 c_2 + h_3 c_3 - c  \right)
$$
where $c_i$ is the phase concentration for phase $i$, $c$ is the physical solute concentration, and $h_i$ is the interpolation function for phase $i$ defined in \cite{Folch05} (referred to as $g_i$ there, but we use $h_i$ to maintain consistency with other interpolation functions in MOOSE).

### Jacobian

#### On-diagonal
Since the non-linear variable for this kernel is $c_3$, the on-diagonal Jacobian is
$$
\begin{eqnarray*}
J &=& \phi_j \frac{\partial R}{\partial c_3} \\
&=& \phi_j h_3
\end{eqnarray*}
$$

#### Off-diagonal
For the physical concentration $c$, the off-diagonal Jacobian is
$$
\begin{eqnarray*}
J &=& \phi_j \frac{\partial R}{\partial c} \\
&=& - \phi_j
\end{eqnarray*}
$$
For phase concentrations other than $c_3$, such as $c_1$:
$$
\begin{eqnarray*}
J &=& \phi_j \frac{\partial R}{\partial c_1} \\
&=& \phi_j h_1
\end{eqnarray*}
$$
and similar for $c_2$.

Finally for the order parameters, such as $\eta_1$,
$$
\begin{eqnarray*}
J &=& \phi_j \frac{\partial R}{\partial \eta_1} \\
&=& \phi_j \left( \frac{\partial h_1}{\partial \eta_1} c_1 + \frac{\partial h_2}{\partial \eta_1} c_2 +  \frac{\partial h_3}{\partial \eta_1} c_3      \right)
\end{eqnarray*}
$$
For the off-diagonal Jacobians we also need to multiply by $L$, the Allen-Cahn mobility.


!parameters /Kernels/KKSMultiPhaseConcentration

!inputfiles /Kernels/KKSMultiPhaseConcentration

!childobjects /Kernels/KKSMultiPhaseConcentration

\bibliography{docs/bib/phase_field.bib}
