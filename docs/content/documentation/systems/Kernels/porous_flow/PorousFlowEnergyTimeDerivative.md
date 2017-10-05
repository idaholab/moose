# PorousFlowEnergyTimeDerivative
!syntax description /Kernels/PorousFlowEnergyTimeDerivative

This `Kernel` implements the weak form of
\begin{equation*}
  \frac{\partial}{\partial t}\left((1-\phi)\rho_{R}C_{R}T + \phi\sum_{\beta}S_{\beta}\rho_{\beta}\mathcal{E}_{\beta}\right)
\end{equation*}
where all parameters are defined in the [nomenclature](/porous_flow/nomenclature.md).

!!! note
    An energy-lumped version is implemented

See [mass lumping](/porous_flow/mass_lumping.md) for details.

!syntax parameters /Kernels/PorousFlowEnergyTimeDerivative

!syntax inputs /Kernels/PorousFlowEnergyTimeDerivative

!syntax children /Kernels/PorousFlowEnergyTimeDerivative
