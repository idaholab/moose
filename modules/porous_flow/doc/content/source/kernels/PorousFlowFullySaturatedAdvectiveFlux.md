# PorousFlowFullySaturatedAdvectiveFlux

!syntax description /Kernels/PorousFlowFullySaturatedAdvectiveFlux

Describes the differential term
\begin{equation}
-\nabla\cdot ((\rho)\chi^{\kappa} k(\nabla P - \rho \mathbf{g})/\mu) \ .
\end{equation}
The nomenclature is described [here](nomenclature.md).  This is fully-saturated, multi-component, single-phase Darcy flow for fluid component $\kappa$.

The factor $(\rho)$ appears in parenthases because it is optional and contolled by the `multiply_by_density` flag.  If `multiply_by_density = false` is chosen then the problem becomes more linear (and hence converges faster) but care must be taken with other PorousFlow objects (such as [PorousFlowFluidMass](PorousFlowFluidMass.md)) since all fluxes become *volume fluxes* (with SI units m$^{3}$.m$^{-3}$.s$^{-1}$) instead of the usual mass fluxes (with SI units kg.m$^{-3}$.s$^{-1}$)

[Full upwinding](upwinding.md) is used in this Kernel.  See the [numerical stabilization lead page](stabilization.md) page for more details.

This Kernel may be added automatically by the [PorousFlowFullySaturated](PorousFlowFullySaturated.md) Action: see more discussion on that page.

!syntax parameters /Kernels/PorousFlowFullySaturatedAdvectiveFlux

!syntax inputs /Kernels/PorousFlowFullySaturatedAdvectiveFlux

!syntax children /Kernels/PorousFlowFullySaturatedAdvectiveFlux

!bibtex bibliography
