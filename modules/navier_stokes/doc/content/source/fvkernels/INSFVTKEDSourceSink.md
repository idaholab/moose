# INSFVTKEDSourceSink

The object computes the turbulent source and sink term for the turbulent kinetic energy dissipation rate equation.

Two terms are computed: `destruction` and `production` and the term `destruction - production` is
passed to the residual.
A different treatment is used for the bulk and the near wall regions.

## Bulk formulation:

The production of turbulent kinetic energy dissipation $G_\epsilon$ is modeled as follows:

\begin{equation}
G_{\epsilon} = C_{1,\epsilon} \frac{\epsilon}{k} G_k \,,
\end{equation}

where:

- $C_{1,\epsilon} = 1.44$ is a closure parameter,
- $G_k$ is the limited turbulent kinetic energy production. For more details please refer to [INSFVTKESourceSink](INSFVTKESourceSink.md).

The destruction of the turbulent kinetic energy dissipation rate is modeled as follows:

\begin{equation}
\epsilon_{\epsilon} = \frac{C_{2,\epsilon} \rho \epsilon}{t_k} \,,
\end{equation}

where:

- $C_{2,\epsilon} = 1.92$ is a closure parameter,
- $\epsilon$ is the solution variable, i.e., the dissipation rate of the turbulent kinetic energy,
- $k$ is the turbulent kinetic energy,
- $t_k = \frac{k}{\epsilon}$ is the turbulent time scale; if the [!param](/FVKernels/INSFVTKEDSourceSink/linearized_model) is `true`, this timescale is computed from the previous iteration; if [!param](/FVKernels/INSFVTKEDSourceSink/linearized_model) is `false`, in a nonlinear solve, this timescale is aded to the Jacobian.

## Wall formulation:

All cells in contact with a boundary identified in the [!param](/FVKernels/INSFVTKEDSourceSink/walls) list are applied a different
treatment in which the equilibrium value for the $\epsilon = \epsilon_{eq}$ is set.
A separate formulation is used for the `sub-laminar` and `logarithmic` boundary layers.
The determination of whether the near-wall cell lies in the laminar or logarithmic region
is performed via the non-dimensional wall distance $y^+$.
The non-dimensional wall distance can be defined differently according to the
[!param](/FVKernels/INSFVTKEDSourceSink/wall_treatment) parameter. 

The four formulations are described in more detail in [INSFVTurbulentViscosityWallFunction.md]. 

If an equilibrium [!param](/FVKernels/INSFVTKEDSourceSink/wall_treatment) is defined, i.e. `eq_newton`,`eq_incremental` or `eq_linearized`, the standard wall function formulations are used in which $y^+$ is found:

\begin{equation}
y^+ = \frac{\rho y_p u_{\tau}}{\mu} \,,
\end{equation}

where:

- $\rho$ is the density,
- $y_p$ is the distance from the wall to the centroid of the next-to-wall cell,
- $u_{\tau}$ is the friction velocity, defined as $u_{\tau} = \sqrt{\frac{\tau_w}{\rho}}$ with $\tau_w$ the shear stress at the wall for which the condition is applied,
- $\mu$ is the dynamic molecular viscosity.

If a non-equilibrium [!param](/FVKernels/INSFVTKEDSourceSink/wall_treatment) is defined, i.e. `neq`,
the $y^+$ is defined non-iteratively as follows:

\begin{equation}
y^+ = \frac{y_p \sqrt{\sqrt{C_{\mu}}k}}{\mu} \,,
\end{equation}

!alert note
Using non-equilibrium wall functions is recommended for problems with recirculations and boundary layer detachment. However, using non-equilibrium wall functions will deteriorate results for standard problems such as flow developing over walls.

The cells with $y^+ < 11.25$ belong to `sub-laminar` boundary layer.
The ones belonging to the `logarithmic` boundary layer are those for which $y^+ \ge 11.25$.

A different value is used for $\epsilon_{eq}$ in each of the two regions.
For the `sub-laminar` boundary layer, the equilibrium value is determined as follows:

\begin{equation}
\epsilon_{eq} = 2 \frac{k \mu}{y_p^2}\,,
\end{equation}

where:

- $\mu_t$ is the turbulent dynamic viscosity.

For the `logarithmic` boundary layer, the value is determined as follows:

\begin{equation}
\epsilon_{eq} = \frac{C_{\mu}^{0.75} \rho k^{1.5}}{\kappa y_p}\,,
\end{equation}

where:

- $\kappa = 0.4187$ is the von Kármán constant.

!alert note
When using wall functions, since the equilibrium value for $\epsilon$ is set in the cells near the wall, the user is recommended to deactivate advection and diffusion for those near wall cells.

!alert note
When the wall treatment is specified in this kernel, any boundary condition for $\epsilon$ will be ignored.
In other words, there is no need to impose boundary conditions for $\epsilon$ when the wall treatment
is specified in his kernel.

!alert note
When using near-wall treatment, we assume that the $\mu_t$ functor is computed by an object
that performs near-wall treatment. Otherwise, the results obtained won't be physically correct.

!syntax parameters /FVKernels/INSFVTKEDSourceSink

!syntax inputs /FVKernels/INSFVTKEDSourceSink

!syntax children /FVKernels/INSFVTKEDSourceSink
