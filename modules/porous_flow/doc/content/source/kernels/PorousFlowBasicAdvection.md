# PorousFlowBasicAdvection

!syntax description /Kernels/PorousFlowBasicAdvection

This Kernel implements the differential expression
\begin{equation}
\nabla\cdot(\mathbf{v}_{\beta} u) \ ,
\end{equation}
where $\mathbf{v}_{\beta}$ is Darcy velocity of fluid phase $\beta$.  That is, the advection of a variable $u$.  The Darcy velocity is defined to be
\begin{equation}
\mathbf{v}_{\beta} = -\frac{k k_{\mathrm{r,}\ \beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta}\mathbf{g}) \ ,
\end{equation}
where $k$ is the permeability tensor, $k_{\mathrm{r,}\ \beta}$ is the relative permeability, $\mu$ is the viscosity, $P$ is the porepressure, $\rho$ is the density and $\mathbf{g}$ is the gravitational acceleration.

This Kernel is designed to be used with tracers or other similar variables that are not described by the mass fraction variables of PorousFlow.  It requires the use of the [`PorousFlowDarcyVelocityMaterial`](PorousFlowDarcyVelocityMaterial.md) material.

!alert warning
The most naive implementation of advection is used.  You will almost definitely see spurious overshoots and undershoots as well as numerical diffusion when using this `Kernel`.  It is usually better to use the [`PorousFlowAdvectiveFlux`](PorousFlowAdvectiveFlux.md) if possible.

The typical usage of this `Kernel` is to model the advection equation $\dot{u}+\nabla (vu) = 0$, which is implemented by the following:

!listing modules/porous_flow/test/tests/basic_advection/1phase.i start=[Kernels] end=[UserObjects]

!syntax parameters /Kernels/PorousFlowBasicAdvection

!syntax inputs /Kernels/PorousFlowBasicAdvection

!syntax children /Kernels/PorousFlowBasicAdvection
