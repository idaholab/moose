# AqueousEquilibriumRxnAux

!syntax description /AuxKernels/AqueousEquilibriumRxnAux

Calculates the concentration of the $i^{\mathrm{th}}$ secondary species, $C_i$ in terms of
primary species concentration $C_j$ using the mass action equation

\begin{equation}
C_i = \frac{K_i}{\gamma_i} \prod_{j=1}^{N_c} \left(\gamma_j C_j\right)^{\nu_{ji}},
\end{equation}
where $K_i$ is equilibrium constant, $\gamma_i$ is the activity coefficient, and
$N_c$ is the number of primary species.

!syntax parameters /AuxKernels/AqueousEquilibriumRxnAux

!syntax inputs /AuxKernels/AqueousEquilibriumRxnAux

!syntax children /AuxKernels/AqueousEquilibriumRxnAux
