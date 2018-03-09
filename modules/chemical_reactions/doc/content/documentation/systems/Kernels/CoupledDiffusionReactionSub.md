# CoupledDiffusionReactionSub

!syntax description /Kernels/CoupledDiffusionReactionSub

Diffusive flux of the $j^{\mathrm{th}}$ primary species present in the secondary equilibrium
species. Implements the weak form of

\begin{equation}
\nabla \cdot \left[\phi D \nabla \left(\sum_i \nu_{ji} C_i\right) \right]
\end{equation}
where $\phi$ is porosity, $D$ is the diffusivity, $\nu_{ji}$ are the stoichiometric coefficients,
$C_i$ is the concentration of the $i^{\mathrm{th}}$ secondary equilibrium species.

!syntax parameters /Kernels/CoupledDiffusionReactionSub

!syntax inputs /Kernels/CoupledDiffusionReactionSub

!syntax children /Kernels/CoupledDiffusionReactionSub
