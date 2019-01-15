# PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat

This `UserObject` implements the [Kuzmin-Turek stabilization scheme](kt.md) for heat advection in the case of multi-phase porous flow.  Specifically, it computes $u$ and $\mathbf{v}$ and $u$ given by
\begin{equation}
u = h k_{r} \rho / \mu \ ,
\end{equation}
and
\begin{equation}
v_{i} = -k_{ij}(\nabla_{j}P - \rho g_{j}) \ ,
\end{equation}
where $k_{ij}$ is the permeability tensor, $P$ is the porepressure of the phase, $\rho$ is the fluid density of the phase, and $g_{j}$ is the acceleration due to gravity, $h$ is the fluid enthalpy of the phase, $k_{r}$ is the relative permeability of the phase, and $\mu$ is the fluid viscosity of the phase.  Derivatives of these quantities with respect to the PorousFlow Variables (defined in the [PorousFlowDictator](PorousFlowDictator.md)) are also computed.

The computed $u$ and $\mathbf{v}$ may then be used by the [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernel to simulate heat flow.  Typically $N_{\beta}$ (number of fluid phases) of such Kernel-UserObject pairs would contribute to the Residual for the temperature variable, corresponding to heat being transferred by each of the fluid phases.  See details on the [Kuzmin-Turek stabilization scheme](kt.md) page.

!syntax parameters /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat

!syntax inputs /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat

!syntax children /UserObjects/PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat
