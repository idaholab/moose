# PorousFlowDarcyVelocityComponentLowerDimensional
!syntax description /AuxKernels/PorousFlowDarcyVelocityComponentLowerDimensional

This `AuxKernel` records the Darcy velocity within a lower-dimensional element living in a higher-dimensional mesh.  For instance, to study flow within a fractured material, you might have created a 3D mesh with its own 3D subdomains (blocks of elements representing different aquifers and aquitards, for example) and within that 3D mesh you might have included 2D elements to represent the fractures.  Those 2D elements must share nodes with the 3D elements for the MOOSE model to make sense.  The 2D elements belong to a different set of subdomains, and those subdomains typically have different Material properties assigned to them (for example, high permeability and porosity).  If you want to measure Darcy velocity within those lower-dimensional subdomains, then use this `AuxKernel`.

This `AuxKernel` calculates the *x*, *y*, or *z* component of the Darcy velocity
\begin{equation*}
  -\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})
\end{equation*}
for fluid phase $\beta$, where $\nabla P_{\beta}$ and $\mathbf{g}$ are projected onto the tangent direction of the lower-dimensional element.
 All parameters are defined in the [nomenclature](/porous_flow/nomenclature.md).

!!! note:
    As this `AuxKernel` uses material properties, only elemental (`Monomial`) `AuxVariables`
    can be used.  The `AuxVariables` must be defined on the lower-dimensional subdomain only.

!!! warning
    For the result to make sense, the permeability tensor, $k$, must not rotate tangential vectors to non-tangential vectors.  For instance, an isotropic permeability tensor is sensible.



!syntax parameters /AuxKernels/PorousFlowDarcyVelocityComponentLowerDimensional

!syntax inputs /AuxKernels/PorousFlowDarcyVelocityComponentLowerDimensional

!syntax children /AuxKernels/PorousFlowDarcyVelocityComponentLowerDimensional
