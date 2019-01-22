# PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent

This `UserObject` implements the [Kuzmin-Turek stabilization scheme](kt.md) in the case of multi-phase, multi-component porous flow.  Specifically, it computes $u$ and $\mathbf{v}$ and $u$ given by
\begin{equation}
u = \chi k_{r} \rho / \mu \ ,
\end{equation}
and
\begin{equation}
v_{i} = -k_{ij}(\nabla_{j}P - \rho g_{j}) \ ,
\end{equation}
where $k_{ij}$ is the permeability tensor, $P$ is the porepressure of the phase, $\rho$ is the fluid density of the phase, and $g_{j}$ is the acceleration due to gravity, $k_{r}$ is the relative permeability of the phase, $\chi$ is the mass fraction of the component in the phase, and $\mu$ is the fluid viscosity of the phase.  Derivatives of these quantities with respect to the PorousFlow Variables (defined in the [PorousFlowDictator](PorousFlowDictator.md)) are also computed.

The computed $u$ and $\mathbf{v}$ may then be used by the [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernel.  $N_{\beta}$ (the number of fluid phases) of these `UserObjects` typically contribute to the residual for a single variable: see details on the [Kuzmin-Turek stabilization scheme](kt.md) page.

!syntax parameters /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent

!syntax inputs /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent

!syntax children /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
