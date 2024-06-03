# LinearWCNSFVMomentumFlux

This kernel adds the contributions of two terms that require face fluxes
in the momentum equations of the incompressible/weakly-compressible Navier Stokes equations:

- Momentum advection term
- Viscous stress term

We discuss these two terms in detail below.

# Momentum advection term

This term is described by the $\nabla \cdot \left(\rho\vec{u} \otimes \vec{u}\right)$
component of the incompressible/weakly-compressible Navier Stokes
momentum equation.

The face mass flux is provided by the [RhieChowMassFlux.md] object which uses pressure
gradients and the discrete momentum equation to compute face velocities and mass fluxes.
For more information on the expression that is used, see [SIMPLE.md].

Once the face flux is given ($(\rho \vec{u}\cdot \vec{n})_{RC} $), the integral of the
advection term over a cell can be expressed as:

\begin{equation}
\int\limits_{V_C} \nabla \cdot \left(\rho\vec{u} \otimes \vec{u}\right) dV \approx \sum\limits_f (\rho \vec{u}\cdot \vec{n})_{RC} \vec{u}_f |S_f| \,
\end{equation}

where `\vec{u}_f` is a face velocity. This face velocity acts as the advected quantity and a linear average or upwind scheme can be used to compute it. This kernel adds the
face contribution for each face $f$ to the right hand side and matrix.

# Viscous stress term

This term is described by the $\nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u} +\nabla \vec{u}^T - \frac{2}{3} \nabla \cdot \vec{u} \mathbb{I} \right)\right)$
component of the incompressible/weakly-compressible Navier Stokes
momentum equation. Using the divergence theorem and the finite volume approximation,
this term can be expressed as

\begin{equation}
\int\limits_{V_C} \nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u} +\nabla \vec{u}^T - \frac{2}{3} \nabla \cdot \vec{u} \mathbb{I} \right)\right) dV
\approx \sum\limits_f \mu_\text{eff} \left(\nabla\vec{u} +\nabla \vec{u}^T - \frac{2}{3} \nabla \cdot \vec{u} \mathbb{I} \right) \cdot \vec{n}_f |S_f| \,
\end{equation}

where the first term ($\mu_\text{eff}\nabla\vec{u} \cdot \vec{n}_f |S_f|$) can be
discretized using the same method as in [LinearFVDiffusion.md]. The other two terms,
($\left(\mu_\text{eff}\nabla\vec{u}^T - \frac{2}{3} \nabla \cdot \vec{u} \mathbb{I} \right)\cdot \vec{n}_f |S_f|$) are treated explicitly meaning that they don't contribute to
the system matrix, only to the right hand side.

For incompressible simulations with constant viscosity fields, the last two terms are
provably 0. Furthermore, in most scenarios, these two terms are negligible compared to
the first term so the user can elect to disable them using [!param](/LinearFVKernels/LinearWCNSFVMomentumFlux/use_deviatoric_terms) parameter.

Similarly to [LinearFVDiffusion.md], once can select to utilize nonorthogonal corrections
for the first term using the [!param](/LinearFVKernels/LinearWCNSFVMomentumFlux/use_nonorthogonal_correction) parameter.


!syntax parameters /LinearFVKernels/LinearWCNSFVMomentumFlux

!syntax inputs /LinearFVKernels/LinearWCNSFVMomentumFlux

!syntax children /LinearFVKernels/LinearWCNSFVMomentumFlux
