# PorousFlowMassVolumetricExpansion

!syntax description /Kernels/PorousFlowMassVolumetricExpansion

This `Kernel` implements the weak form of
\begin{equation*}
  \phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}\nabla\cdot\mathbf{v}_{s}
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).

!alert note
A mass-lumped version is implemented.

See [mass lumping](/mass_lumping.md) for details.

The multiplication by $\rho_{\beta}$ is optional and is controlled by the `multiply_by_density` flag.  It is sometimes advantageous to use this flag because the problem becomes more linear.  However, this sometimes changes the nature of the physical problem modelled, and care must be taken when using other PorousFlow objects (that intrinsically have `multiply_by_density = true`, such as [PorousFlowFluidMass](PorousFlowFluidMass.md)) so new users are encouraged to use the default `multiply_by_density = true` flag until they gain familiarity with PorousFlow.

!syntax parameters /Kernels/PorousFlowMassVolumetricExpansion

!syntax inputs /Kernels/PorousFlowMassVolumetricExpansion

!syntax children /Kernels/PorousFlowMassVolumetricExpansion
