# CoupledConvectionReactionSub

!syntax description /Kernels/CoupledConvectionReactionSub

Convective flux of concentration of the $j^{\mathrm{th}}$ primary species present in the secondary
equilibrium species. Implements the weak form of

\begin{equation}
\mathbf{q} \cdot \left[ \nabla \left(\sum_i \nu_{ji} C_i\right) \right]
\end{equation}
where $\nu_{ji}$ are the stoichiometric coefficients, $C_i$ is the concentration of the
$i^{\mathrm{th}}$ secondary equilibrium species, and $\mathbf{q}$ is the Darcy velocity

\begin{equation}
\mathbf{q} = - \frac{K}{\mu} \left(\nabla P - \rho \mathbf{g}\right),
\end{equation}
where $K$ is the permeability tensor, $\mu$ is fluid viscosity, $P$ is pressure, $\rho$ is fluid
density, and $\mathbf{g}$ is gravity (by default, the gravity term is not included).

!alert note
$K/\mu$ and $\rho$ are expected as material properties called conductivity and density, respectively.

!syntax parameters /Kernels/CoupledConvectionReactionSub

!syntax inputs /Kernels/CoupledConvectionReactionSub

!syntax children /Kernels/CoupledConvectionReactionSub
