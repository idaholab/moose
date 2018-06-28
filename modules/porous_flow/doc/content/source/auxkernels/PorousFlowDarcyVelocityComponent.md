# PorousFlowDarcyVelocityComponent

!syntax description /AuxKernels/PorousFlowDarcyVelocityComponent

This `AuxKernel` calculates the *x*, *y*, or *z* component of the Darcy velocity
\begin{equation*}
  -\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})
\end{equation*}
for fluid phase $\beta$. All parameters are defined in the [nomenclature](/nomenclature.md).

!alert note
As this `AuxKernel` uses material properties, only elemental (`Monomial`) `AuxVariables`
can be used.

!syntax parameters /AuxKernels/PorousFlowDarcyVelocityComponent

!syntax inputs /AuxKernels/PorousFlowDarcyVelocityComponent

!syntax children /AuxKernels/PorousFlowDarcyVelocityComponent
