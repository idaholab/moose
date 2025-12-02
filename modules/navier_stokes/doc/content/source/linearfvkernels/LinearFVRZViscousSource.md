# LinearFVRZViscousSource

!syntax description /LinearFVKernels/LinearFVRZViscousSource

This kernel supplies the axisymmetric viscous source term $\mu u_r / r^2$ that appears in
the cylindrical vector Laplacian.

When [!param](/LinearFVKernels/LinearFVRZViscousSource/use_deviatoric_terms) is enabled, the
axisymmetric contribution is modified to include the $-\frac{2}{3} \nabla\cdot \boldsymbol{u}$
portion of the deviatoric stress.  This doubles the implicit coefficient multiplying
$u_r / r^2$ and adds a right-hand-side contribution of $\frac{2}{3}\mu \nabla\cdot \boldsymbol{u}/r$.
Because the divergence is evaluated from the velocity gradients, you must also provide the names of
each velocity component through the [!param](/LinearFVKernels/LinearFVRZViscousSource/u) and
[!param](/LinearFVKernels/LinearFVRZViscousSource/v) parameters whenever the deviatoric terms are
requested.

This kernel must operate on the radial velocity component only; attempting to apply it to a
non-radial component results in a parameter error. In coupled RZ simulations, enable this kernel's
[!param](/LinearFVKernels/LinearFVRZViscousSource/use_deviatoric_terms) together with
[!param](/LinearFVKernels/LinearWCNSFVMomentumFlux/use_deviatoric_terms) in [LinearWCNSFVMomentumFlux.md] so that the combination of
these kernels reproduces the full cylindrical viscous
operator (including the explicit hoop-stress contribution discussed in the latter's documentation).

## RZ coordinate system

For axisymmetric, swirl-free flows we take $\vec{u} = (u_r, u_z)$ and
\begin{equation}
\nabla \cdot \vec{u} = \frac{\partial u_r}{\partial r} + \frac{u_r}{r} + \frac{\partial u_z}{\partial z}.
\end{equation}
The Newtonian stress tensor with the Stokes approximation,
$\boldsymbol{\tau} = \mu \nabla \vec{u} + \mu (\nabla \vec{u})^{\top} - \tfrac{2}{3}\mu (\nabla\cdot\vec{u})\mathbf{I}$,
then contains a hoop component $\tau_{\theta\theta} = 2\mu u_r / r - \tfrac{2}{3}\mu \nabla\cdot\vec{u}$.
Taking the divergence produces the volumetric contribution
$-(\tau_{\theta\theta}/r)\,\mathbf{e}_r \approx - 2\mu u_r/r^2 + \tfrac{2}{3}\mu (\nabla\cdot\vec{u})/r$
that this kernel supplies, while the face terms are handled by
[LinearWCNSFVMomentumFlux.md].

For more information on the axisymmetric formulation we recommend visiting [!cite](peterson2018overview) and
[!cite](hill2018note).

!syntax parameters /LinearFVKernels/LinearFVRZViscousSource

!syntax inputs /LinearFVKernels/LinearFVRZViscousSource

!syntax children /LinearFVKernels/LinearFVRZViscousSource
