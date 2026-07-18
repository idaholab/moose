# PorousFlowFullySaturatedUpwindHeatAdvection

!syntax description /Kernels/PorousFlowFullySaturatedUpwindHeatAdvection

This `Kernel` implements the weak form of
\begin{equation*}
  -\nabla\cdot h \rho\frac{k}{\mu}(\nabla P - \rho \mathbf{g})
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).  This is only relevant for situations involving a single-phase fluid: if you have a multi-phase scenario you will want to use [PorousFlowHeatAdvection](PorousFlowHeatAdvection.md).

This `Kernel` is usually added by the [PorousFlowFullySaturated](PorousFlowFullySaturated.md) Action (and more discussion is included on that page).

!alert note
A fully-upwinded version is implemented, where the mobility of the upstream nodes is used.  This Kernel requires a nodal (Lagrange) variable, because upwinding associates each element degree of freedom with a mesh node; using a non-nodal variable (such as a monomial variable) is an error.

See [upwinding](/upwinding.md) for details.  If you desire no upwinding, use [PorousFlowFullySaturatedHeatAdvection](PorousFlowFullySaturatedHeatAdvection.md) instead.  If you desire [Kuzmin-Turek TVD stabilization](kt.md), use [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md).

!syntax parameters /Kernels/PorousFlowFullySaturatedUpwindHeatAdvection

!syntax inputs /Kernels/PorousFlowFullySaturatedUpwindHeatAdvection

!syntax children /Kernels/PorousFlowFullySaturatedUpwindHeatAdvection
