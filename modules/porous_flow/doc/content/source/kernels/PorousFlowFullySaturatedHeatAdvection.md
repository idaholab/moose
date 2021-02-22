# PorousFlowFullySaturatedHeatAdvection

!syntax description /Kernels/PorousFlowFullySaturatedHeatAdvection

Describes the differential term
\begin{equation}
-\nabla\cdot ((\rho)h k(\nabla P - \rho \mathbf{g})/\mu) \ .
\end{equation}
The nomenclature is described [here](nomenclature.md).  This describes heat advection via Darcy flow in fully-saturated, multi-component, single-phase cases.

!alert note
Although the multiplication by $\rho$ is optional, you should almost always set `multiply_by_density=true`

!alert note
No [upwinding](upwinding.md) is performed, which means many [nodal Material properties](tutorial_09.md) are not needed and [numerical diffusion](numerical_diffusion.md) is reduced.  However, the numerics are less well controlled: the whole point of full upwinding is to prevent over-shoots and under-shoots in the temperature, etc.

If you desire full upwinding, use [PorousFlowFullySaturatedUpwindHeatAdvection](PorousFlowFullySaturatedUpwindHeatAdvection.md) instead.  If you desire [Kuzmin-Turek TVD stabilization](kt.md), use [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md).


!syntax parameters /Kernels/PorousFlowFullySaturatedHeatAdvection

!syntax inputs /Kernels/PorousFlowFullySaturatedHeatAdvection

!syntax children /Kernels/PorousFlowFullySaturatedHeatAdvection

!bibtex bibliography
