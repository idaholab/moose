# INSFVTKEDSourceSink

!syntax description /FVKernels/INSFVTKEDSourceSink

The object computes the turbulent source and sink term for the turbulent kinetic energy equation.

Two terms are computed `destruction` and `production` and the term `destruction - production` is
passed to the residual.
A different treatment is used for the bulk and the near wall regions.

*Bulk formulation*:

The turbulent production $G_k$ is modeled as follows:

\begin{equation}
G_{\epsilon} = C_{1,\epsilon} \rho C_{\mu} S^2 k \,,
\end{equation}

where:

- $C_{1,\epsilon} = 1.44$ is a closure parameter,
- $\rho$ is the density
- $C_{\mu} = 0.09$ is another closure parameter,
- $S$ is the shear strain tensor internal norm, defined as $S = \sqrt{2\mathbf{S}:\mathbf{S}}$ with the shear strain tensor defined as $\mathbf{S} = \frac{1}{2} [\nabla \vec{u} + (\nabla \vec{u})^T]$,
- $k$ is the turbulent kinetic energy, which generally comes from a coupled transport equation or can be set by the user for reproducing canonical cases.

The dissipation rate of the turbulent kinetic energy dissipation rate is modeled as follows:

\begin{equation}
\epsilon_{\epsilon} = \frac{C_{2,\epsilon} \rho \epsilon}{t_k} \,,
\end{equation}

where:

- $C_{2,\epsilon} = 1.92$ is a closure parameter,
- $\epsilon$ is the solution variable, i.e., the dissipation rate of the turbulent kinetic energy dissipation rate,
- $t_k = \frac{k}{\epsilon}$ is the turbulent time scale; if the `linearized_model = true`, this timescale is computed from the previous iteration; if `linearized_model = false`, in a nonlinear solve, this timescale is aded to the Jacobian.

*Wall formulation*:

All cells in contact with a boundary identified in the `walls` list are applied a different
treatment in which the equilibrium value for the $\epsilon = \epsilon_{eq}$ is set.
A separate formulation is used for the `sub-laminar` and `logarithmic` boundary layers.
The determination of whether the near-wall cell lies in the laminar or logarithmic region
is performed via the non-dimensional wall distance $y^+$.
The non-dimensional wall distance is defined as defined differently according to the
`non_equilibrium_treatement`.

If `non_equilibrium_treatement = false`, the standard wall function formulations is used in
which the $y^+$ is found by an incremental fixed point algorithm as follows:

\begin{equation}
y^+ = \frac{\rho y_p u_{\tau}}{\mu} \,,
\end{equation}

where:

- $\rho$ is the density,
- $y_p$ is the distance to the wall to the centroid of the next-to-wall cell,
- $u_{\tau}$ is the friction velocity, defined as $u_{\tau} = \sqrt{\frac{\tau_w}{\rho}}$ with $\tau_w$ the shear stress at the wall for which the condition is applied,
- $\mu$ is the dynamic molecular viscosity.

If `non_equilibrium_treatement = true`, non-equilibrium wall function formulations is
used in which the $y^+$ is defined as follows:

\begin{equation}
y^+ = \frac{C_{\mu}^{0.25} y_p \sqrt{k}}{\mu} \,,
\end{equation}

!alert note
Using non-equilibrium wall functions is recomended for problems with recirculations and boundary layer detachment. However, using non-equilibrium wall functions will deteriorate results for standard problems such as flow developing over walls.

The cells belonging to the `sub-laminar` boundary layers are defined as those
for which $y^+ < 11.25$.
The ones belonging to the `logarithmic` boundary layer are those for which $y^+ \ge 11.25$.

A different value is used for $\epsilon_{eq}$ in each of the two regions.
For the `sub-laminar` boundary layer, the equilibrium value is determined as follows:

\begin{equation}
\epsilon_{eq} = 2 \frac{k \mu_t}{y_p^2}\,,
\end{equation}

where:

- $\mu_t$ is the turbulent dynamic viscosity.

For the `logarithmic` boundary layer, the value is determined as follows:

\begin{equation}
\epsilon_{eq} = \frac{C_{\mu}^{0.75} \rho k^{1.5}}{\kappa y_p}\,,
\end{equation}

where:

- $\kappa = 0.41$ is the von Kármán constant.

!alert note
When using wall functions, since the equilibrium value for $\epsilon$ is set in the cells near the wall, the user is recomended to deactivate advection and diffusion for those near wall cells.

!alert note
When the wall treatment is specified in this kernel, any boundary condition for $\epsilon$ will be ignored.
In other words, there is no need to impose boundary conditions for $\epsilon$ when the wall treatment
is specified in his kernel.

!alert note
When using near-wall treatment, we assume that the $\mu_t$ funtor is computed by an object
that performs near-wall treatment. Otherwise, the results obtained won't not physically correct

!syntax parameters /FVKernels/INSFVTKEDSourceSink

!syntax inputs /FVKernels/INSFVTKEDSourceSink

!syntax children /FVKernels/INSFVTKEDSourceSink
