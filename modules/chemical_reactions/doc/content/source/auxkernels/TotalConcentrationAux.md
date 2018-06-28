# TotalConcentrationAux

!syntax description /AuxKernels/TotalConcentrationAux

Calculates the total concentration of the $j^{\mathrm{th}}$ primary species
as the sum of its free concentration $C_j$ and its stoichiometric contribution
to all secondary equilibrium species in which it participates
\begin{equation}
C_{j, \mathrm{tot}} = C_j + \sum_i \nu_{ji} C_i,
\end{equation}
where $\nu_{ji}$ is the stoichiometric coefficient of the $j^{\mathrm{th}}$ primary
species in the $i^{\mathrm{th}}$ secondary equilibrium species.

!syntax parameters /AuxKernels/TotalConcentrationAux

!syntax inputs /AuxKernels/TotalConcentrationAux

!syntax children /AuxKernels/TotalConcentrationAux
