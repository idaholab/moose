# PorousFlowAdvectiveFluxCalculatorUnsaturated

This `UserObject` implements the [Kuzmin-Turek stabilization scheme](kt.md) in the case of one-phase, one-component, unsaturated Darcy-Richards flow.  Specifically, it computes $u$ and $\mathbf{v}$ and $u$ given by
\begin{equation}
u = k_{r} \rho / \mu \ ,
\end{equation}
and
\begin{equation}
v_{i} = -k_{ij}(\nabla_{j}P - \rho g_{j}) \ ,
\end{equation}
where $k_{ij}$ is the permeability tensor, $P$ is the porepressure, $\rho$ is the fluid density, and $g_{j}$ is the acceleration due to gravity, $k_{r}$ is the relative permeability, and $\mu$ is the fluid viscosity.  Derivatives of these quantities with respect to the PorousFlow Variables (defined in the [PorousFlowDictator](PorousFlowDictator.md)) are also computed.

This `UserObject` may also be used for multi-phase, multi-component porous-flow, if each mass fraction exists in one phase only (so that the mass fractions are just 1 or 0).

The computed $u$ and $\mathbf{v}$ may then be used by the [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernel.

!syntax parameters /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturated

!syntax inputs /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturated

!syntax children /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturated
