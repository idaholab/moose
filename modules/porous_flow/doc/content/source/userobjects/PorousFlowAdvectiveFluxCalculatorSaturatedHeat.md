# PorousFlowAdvectiveFluxCalculatorSaturatedHeat

This `UserObject` implements the [Kuzmin-Turek stabilization scheme](kt.md) for heat advection in the case of single-phase porous flow.  Specifically, it computes $u$ and $\mathbf{v}$ and $u$ given by
\begin{equation}
u = h \rho / \mu \ ,
\end{equation}
and
\begin{equation}
v_{i} = -k_{ij}(\nabla_{j}P - \rho g_{j}) \ ,
\end{equation}
where $k_{ij}$ is the permeability tensor, $P$ is the porepressure, $\rho$ is the fluid density, and $g_{j}$ is the acceleration due to gravity, $h$ is the fluid enthalpy, and $\mu$ is the fluid viscosity.  Derivatives of these quantities with respect to the PorousFlow Variables (defined in the [PorousFlowDictator](PorousFlowDictator.md)) are also computed.

The computed $u$ and $\mathbf{v}$ may then be used by the [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernel to simulate heat flow.  See details on the [Kuzmin-Turek stabilization scheme](kt.md) page.

!syntax parameters /UserObjects/PorousFlowAdvectiveFluxCalculatorSaturatedHeat

!syntax inputs /UserObjects/PorousFlowAdvectiveFluxCalculatorSaturatedHeat

!syntax children /UserObjects/PorousFlowAdvectiveFluxCalculatorSaturatedHeat
