<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# ComputeLinearElasticPFFractureStress
!syntax description /Materials/ComputeLinearElasticPFFractureStress
This material defines the tensile (positive) and compressive (negative) parts of the strain energy, and their derivatives. It also computes the tensile and compressive parts of the stress. It assumes linear elasticity, but that elasticity tensor can be anisotropic. The main ideas are shown, below:
1, Calculate the undamaged stress and then decompose the stress into positive and negative parts;
$$
\begin{eqnarray}
  \bf{\sigma} = \mathbf{C} \bf{\epsilon} = \bf{Q} \bf{\Lambda} \bf{Q^{T}} \\
   = \bf{Q} \bf{\Lambda^{+}}\bf{Q^{T}} + \bf{Q} \bf{\Lambda^{-}}\bf{Q^{T}} \\
   = \bf{\sigma_{0}^{+}} + \bf{\sigma_{0}^{-}}
\end{eqnarray}
$$
2, Get the positive and negative parts of the strain energy, which are induced by the positive and negative parts of the stress we determined in the first step;
$$
\begin{eqnarray}
   \psi^{+} &=& \frac{1}{2} \bf{\sigma_{0}^{+}} : \bf{\epsilon}\\
   \psi^{-} &=& \frac{1}{2} \bf{\sigma_{0}^{-}} : \bf{\epsilon}
\end{eqnarray}
$$
3, Get the total free energy for damaged material;
\begin{eqnarray}
  F &=& [(1-c)^2(1-k) + k] \, H +\psi^{-} + \frac{g_c}{2l}c^2 + \frac{g_c l}{2} {|{\nabla c}|}^2 \\
  &=& f_{loc} + \frac{g_c l}{2} {|{\nabla c}|}^2
\end{eqnarray}
Where H is the history variable defined as the maximum positive strain energy up to current time step.
4, This method is thermodynamically consistent, and the evolution equation for the damage parameter follows the Allen-Cahn equation
\begin{equation}
 \dot{c} = -L \frac{\delta F}{\delta c} = -L \left( \frac{\partial f_{loc}}{\partial c} - \nabla \cdot \kappa \nabla c \right).
\end{equation}
To keep consistency with Miehe's evolution equation, here the parameters in the equation are defined as $ L = (g_c \eta)^{-1}$ and $\kappa = g_c l$. \\

!syntax parameters /Materials/ComputeLinearElasticPFFractureStress
`G0_pos` Positive part of strain energy
`F` Total free energy
`dFdc` Derivative of free energy wrt c
`d2Fdc2` Second derivative of free energy wrt c
`d2Fdcdstrain` Second partial derivative of free energy wrt c and strain tensor
`H0_pos` History variable
`dH0_pos_dstrain` Derivative of positive part of history variable wrt strain tensor
!syntax inputs /Materials/ComputeLinearElasticPFFractureStress

!syntax children /Materials/ComputeLinearElasticPFFractureStress
