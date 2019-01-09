# PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent

This `UserObject` implements the [Kuzmin-Turek stabilization scheme](kt.md) in the case of one-phase, multi-component, fully-saturated Darcy flow.  Specifically, it computes $u$ and $\mathbf{v}$ and $u$ given by
\begin{equation}
u = \chi \rho / \mu \ ,
\end{equation}
and
\begin{equation}
v_{i} = -k_{ij}(\nabla_{j}P - \rho g_{j}) \ ,
\end{equation}
where $k_{ij}$ is the permeability tensor, $P$ is the porepressure, $\rho$ is the fluid density, and $g_{j}$ is the acceleration due to gravity, $\chi$ is the appropriate mass fraction, and $\mu$ is the fluid viscosity.  Derivatives of these quantities with respect to the PorousFlow Variables (defined in the [PorousFlowDictator](PorousFlowDictator.md)) are also computed.

The computed $u$ and $\mathbf{v}$ may then be used by the [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernel.

!syntax parameters /UserObjects/PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent

!syntax inputs /UserObjects/PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent

!syntax children /UserObjects/PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent
