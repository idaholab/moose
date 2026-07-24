# PorousFlowMassRadioactiveDecay

!syntax description /Kernels/PorousFlowMassRadioactiveDecay

This `Kernel` implements the weak form of
\begin{equation*}
  \Lambda \phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).

!alert note
A mass-lumped version is implemented.  This Kernel requires a nodal (Lagrange) variable, because mass lumping associates each element degree of freedom with a mesh node; using a non-nodal variable (such as a monomial variable) is an error.

See [mass lumping](/mass_lumping.md) for details.

!syntax parameters /Kernels/PorousFlowMassRadioactiveDecay

!syntax inputs /Kernels/PorousFlowMassRadioactiveDecay

!syntax children /Kernels/PorousFlowMassRadioactiveDecay
