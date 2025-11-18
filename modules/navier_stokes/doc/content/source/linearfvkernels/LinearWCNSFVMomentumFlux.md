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

## Axisymmetric (RZ) considerations

When the kernel operates on blocks that use the `COORD_RZ` coordinate system, the stress term
must incorporate the hoop contribution that arises from the cylindrical metric. In practice this
means:

- The divergence $\nabla \cdot \vec{u}$ that appears in the explicit
  $\left(\nabla \vec{u}^T - \tfrac{2}{3} \nabla \cdot \vec{u} \mathbb{I}\right)$ term is
  augmented with the usual $u_r / r$ contribution so the axisymmetric trace is formed exactly on
  every face.
- The implicit part of $\mu \nabla \vec{u}$ still handles only the normal-gradient piece. The
  remaining volumetric contribution $2 \mu u_r / r^2$ is supplied by
  [LinearFVRZViscousSource.md]. When [!param](/LinearFVKernels/LinearWCNSFVMomentumFlux/use_deviatoric_terms)
  is enabled (and likewise
  [!param](/LinearFVKernels/LinearFVRZViscousSource/use_deviatoric_terms)), the source kernel also
  injects the $-\tfrac{2}{3} \mu \nabla \cdot \vec{u} / r$ correction so the combination of the
  two kernels reproduces the full cylindrical viscous operator.

Therefore, for MMS problems or physics runs with space-dependent viscosities in RZ, enable
[!param](/LinearFVKernels/LinearWCNSFVMomentumFlux/use_deviatoric_terms) together with
[!param](/LinearFVKernels/LinearFVRZViscousSource/use_deviatoric_terms) so the explicit hoop stress
matches the analytic forcing.


!syntax parameters /LinearFVKernels/LinearWCNSFVMomentumFlux

!syntax inputs /LinearFVKernels/LinearWCNSFVMomentumFlux

!syntax children /LinearFVKernels/LinearWCNSFVMomentumFlux
