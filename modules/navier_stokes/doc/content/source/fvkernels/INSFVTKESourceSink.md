# INSFVTKESourceSink

!syntax description /FVKernels/INSFVTKESourceSink

The object computes the turbulent source and sink term for the turbulent kinetic energy equation.

Two terms are computed `destruction` = $\epsilon$ and `production` = $G_k$ and the term $\epsilon - G_k$ is
passed to the residual.
A different treatment is used for the bulk and the near wall regions.

## Bulk formulation:

The turbulent production $G_k$ is modeled as:

\begin{equation}
G_k = \mu_t S^2 \,,
\end{equation}

where:

- $\mu_t$ is the turbulent dynamic viscosity,
- $S$ is the shear strain tensor internal norm, defined as $S = \sqrt{2\mathbf{S}:\mathbf{S}}$ with the shear strain tensor defined as $\mathbf{S} = \frac{1}{2} [\nabla \vec{u} + (\nabla \vec{u})^T]$.

The turbulent kinetic energy dissipation rate $\epsilon$ is generally coming from a coupled
transport equation for $\epsilon$.
However, for canonical or meassured cases, e.g., isotropic decaying turbulece,
the user can utilize predefined fields through functors in MOOSE.

## Wall formulation:

All cells in contact with a boundary identified in the `walls` list are applied a different
treatment for production and destruction.
A different formulation is used for the `sub-laminar` and `logarithmic` boundary layers.
The determination of whether the near-wall cell lies in the laminar or logarithmic region
is performed via the non-dimensional wall distance $y^+$.
The non-dimensional wall distance is defined as

\begin{equation}
y^+ = \frac{\rho y_p u_{\tau}}{\mu} \,,
\end{equation}

where:

- $\rho$ is the density,
- $y_p$ is the distance to the wall to the centroid of the next-to-wall cell,
- $u_{\tau}$ is the friction velocity, defined as $u_{\tau} = \sqrt{\frac{\tau_w}{\rho}}$ with $\tau_w$ the shear stress at the wall for which the condition is applied,
- $\mu$ is the dynamic molecular viscosity.

For every next-to-wall cell and every iteration step, $y^+$ is found via an
incremental fixed-point search algorithm.
The cells belonging to the `sub-laminar` boundary layers are defined as those
for which $y^+ < 11.25$.
The ones belonging to the `logarithmic` boundary layer are those for which $y^+ \ge 11.25$.

For both the `sub-laminar` and `logarithmic` boundary layers the production term is defined as:

\begin{equation}
G_k = C_{\mu}^{0.25} \mu_t \frac{k^{\frac{3}{2}} ||\nabla \vec{u}||}{\kappa y_p} \,,
\end{equation}

where:

- $C_{\mu} = 0.09$ is a closure parameter,
- $k$ is the turbulent kinetic energy,
- $||\nabla \vec{u}||$ is the near wall velocity gradient norm, which is defined as $$||\nabla \vec{u}||$ = (\nabla \vec{u} \cdot \hat{n}) \cdot \hat{n}$,
- $\kappa = 0.41$ is the von Kármán constant.

The formulation assumes that the near wall value is already imposed in the $\mu_t$ functor.

When solving a linear problem, instead of the nonlinear formulation, the production term is formulated as:

\begin{equation}
G_k = C_{\mu}^{0.25} \mu_t \frac{k \sqrt{k_{old}}||\nabla \vec{u}||}{\kappa y_p} \,.
\end{equation}

where:

- $k_{old}$ is the value of the turbulent kinetic energy in the previous iteration.

Whereas the linear or nonlinear formulation is used is controled by the
[!param](/FVKernels/INSFVTKESourceSink/linearized_model) parameter.

For the destruction formulation is different for the `sub-laminar` and `logarithmic` layers.
For the `sub-laminar` layer, the destruction is defined as follows:

\begin{equation}
\epsilon = \frac{2 \mu_t k}{y_p ^2} \,.
\end{equation}

For the `logarithmic` layer, the destruction is defined as follows:

\begin{equation}
\epsilon = C_{\mu}^{0.75} \frac{\rho k^{\frac{3}{2}}}{\kappa y_p} \,.
\end{equation}

!alert note
When the wall treatment is specified in this kernel, any boundary condition for $k$ will be ignored.
In other words, there is no need to impose boundary conditions for $k$ when the wall treatment
is specified in his kernel.

!alert note
When using near-wall treatment, we assume that the $\mu_t$ funtor is computed by an object
that performs near-wall treatment.
Otherwise, the results obtained won't not physically correct

!syntax parameters /FVKernels/INSFVTKESourceSink

!syntax inputs /FVKernels/INSFVTKESourceSink

!syntax children /FVKernels/INSFVTKESourceSink
