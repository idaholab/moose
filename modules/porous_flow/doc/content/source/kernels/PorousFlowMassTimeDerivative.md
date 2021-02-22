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

The multiplication by $\rho_{\beta}$ is optional and is controlled by the `multiply_by_density` flag.  It is sometimes advantageous to use this flag because the problem becomes more linear.  However, this sometimes changes the nature of the physical problem modelled (consider, for instance, a fully-saturated multicomponent system, where `multiply_by_density = false` implies the steady-state Laplace equation for the porepressure), and care must be taken when using other PorousFlow objects (that intrinsically have `multiply_by_density = true`, such as [PorousFlowFluidMass](PorousFlowFluidMass.md)) so new users are encouraged to use the default `multiply_by_density = true` flag until they gain familiarity with PorousFlow.

!syntax parameters /Kernels/PorousFlowMassTimeDerivative

!syntax inputs /Kernels/PorousFlowMassTimeDerivative

!syntax children /Kernels/PorousFlowMassTimeDerivative
