# PorousFlowFullySaturatedDarcyBase

!syntax description /Kernels/PorousFlowFullySaturatedDarcyBase

Describes the differential term
\begin{equation}
-\nabla\cdot ((\rho)k(\nabla P - \rho \mathbf{g})/\mu) \ .
\end{equation}
The nomenclature is described [here](nomenclature.md).  This is fully-saturated, single-component, single-phase Darcy flow.

The multiplication by $\rho$ is optional: this is indicated by the parenthases.  The reason for this is that the time-derivative part described by [PorousFlowFullySaturatedMassTimeDerivative](PorousFlowFullySaturatedMassTimeDerivative.md) linearises if the total differential equation is not multiplied by density.  Please see that page for a full description of the effects of not multiplying by density.

!alert note
No [upwinding](upwinding.md) is performed, which means many [nodal Material properties](tutorial_09.md) are not needed and [numerical diffusion](numerical_diffusion.md) is reduced.  However, the numerics are less well controlled: the whole point of full upwinding is to prevent over-shoots and under-shoots.    Other Kernels implement [Kuzmin-Turek TVD stabilization](kt.md).

!syntax parameters /Kernels/PorousFlowFullySaturatedDarcyBase

!syntax inputs /Kernels/PorousFlowFullySaturatedDarcyBase

!syntax children /Kernels/PorousFlowFullySaturatedDarcyBase

!bibtex bibliography
