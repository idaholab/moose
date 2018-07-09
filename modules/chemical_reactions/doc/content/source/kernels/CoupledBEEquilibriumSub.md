# CoupledBEEquilibriumSub

!syntax description /Kernels/CoupledBEEquilibriumSub

Derivative of concentration of $j^{\mathrm{th}}$ primary species present in the
secondary equilibrium species wrt time. Implements the weak form of
\begin{equation}
\frac{\partial}{\partial t} \left(\phi \sum_i \nu_{ji} C_i\right),
\end{equation}
where $\phi$ is porosity, $\nu_{ji}$ are the stoichiometric coefficients, and
$C_i$ is the concentration of the $i^{\mathrm{th}}$ secondary equilibrium species.

!syntax parameters /Kernels/CoupledBEEquilibriumSub

!syntax inputs /Kernels/CoupledBEEquilibriumSub

!syntax children /Kernels/CoupledBEEquilibriumSub
