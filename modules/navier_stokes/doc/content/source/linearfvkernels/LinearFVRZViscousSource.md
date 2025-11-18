# LinearFVRZViscousSource

!syntax description /LinearFVKernels/LinearFVRZViscousSource

This kernel supplies the axisymmetric viscous source term $\mu u_r / r^2$ that appears in
the cylindrical vector Laplacian.

When [!param](/LinearFVKernels/LinearFVRZViscousSource/use_deviatoric_terms) is enabled, the
axisymmetric contribution is modified to include the $-\frac{2}{3} \nabla\cdot \boldsymbol{u}$
portion of the deviatoric stress.  This doubles the implicit coefficient multiplying
$u_r / r^2$ and adds a right-hand-side contribution of $\frac{2}{3}\mu \nabla\cdot \boldsymbol{u}/r$.
Because the divergence is evaluated from the velocity gradients, you must also provide the names of
each velocity component through the [!param](/LinearFVKernels/LinearFVRZViscousSource/u),
[!param](/LinearFVKernels/LinearFVRZViscousSource/v), and
[!param](/LinearFVKernels/LinearFVRZViscousSource/w) parameters whenever the deviatoric terms are
requested.

This kernel must operate on the radial velocity component only; attempting to apply it to a
non-radial component results in a parameter error. In coupled RZ simulations, enable
[!param](/LinearFVKernels/LinearFVRZViscousSource/use_deviatoric_terms) together with
[!param](/LinearFVKernels/LinearWCNSFVMomentumFlux/use_deviatoric_terms) so that the combination of
`LinearFVRZViscousSource` and `LinearWCNSFVMomentumFlux` reproduces the full cylindrical viscous
operator (including the explicit hoop-stress contribution discussed in the latter's documentation).

!syntax parameters /LinearFVKernels/LinearFVRZViscousSource

!syntax inputs /LinearFVKernels/LinearFVRZViscousSource

!syntax children /LinearFVKernels/LinearFVRZViscousSource
