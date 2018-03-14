# PorousFlowMassTimeDerivative

!syntax description /Kernels/PorousFlowMassTimeDerivative

This `Kernel` implements the weak form of
\begin{equation*}
  \frac{\partial}{\partial t}\left(\phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}\right)
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).

!alert note
A mass-lumped version is implemented.

See [mass lumping](/mass_lumping.md) for details.

!syntax parameters /Kernels/PorousFlowMassTimeDerivative

!syntax inputs /Kernels/PorousFlowMassTimeDerivative

!syntax children /Kernels/PorousFlowMassTimeDerivative
