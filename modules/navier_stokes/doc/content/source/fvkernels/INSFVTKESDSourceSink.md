# INSFVTKESDSourceSink

The object computes the turbulent source and sink term for the turbulent kinetic energy specific dissipation rate ($\omega$) equation.

Three terms are computed: `destruction`, `production`, and `cross_diffusion` and the term `destruction - production - cross_diffusion` is
passed to the residual.
A different treatment is used for the bulk and the near wall regions.

## Bulk formulation:

The production of turbulent kinetic energy specifc dissipation $G_\omega$ is modeled as follows:

\begin{equation}
G_{\omega} = \rho \gamma S^2 \,,
\end{equation}

where:

- $\rho$ is the density,
- $S$ is the shear strain tensor internal norm, defined as $S = \sqrt{2\mathbf{S}:\mathbf{S}}$ with the shear strain tensor defined as $\mathbf{S} = \frac{1}{2} [\nabla \vec{u} + (\nabla \vec{u})^T]$.
- $\gamma$ is a blending function, which takes different values for the standard and low-Reynold-number formulation of the model (the low-Reynolds version of the model is activated with the [!param](/FVKernels/INSFVTKESourceSink/low_Re_modification) parameter).

*Standard formulation*:

\begin{equation}
\gamma = F_1 \gamma_1 + (1 - F_1) \gamma_2\,,
\end{equation}

where:

- $F_1$ is a blending function, which should be computed using [kOmegaSSTF1BlendingAux](kOmegaSSTF1BlendingAux.md),
- $\gamma_1 = \frac{\beta_1}{\beta^*} - \sigma_{\omega, 1} \frac{\kappa^2}{\sqrt{\beta^*}}$ is a closure parameter with closure coefficientis, $\beta_1 = 0.075$, $\beta^* = 0.09$, $\sigma_{\omega, 1} = 0.5$, and $\kappa = 0.41$,
- $\gamma_2 = \frac{\beta_2}{\beta^*} - \sigma_{\omega, 2} \frac{\kappa^2}{\sqrt{\beta^*}}$ is a closure parameter with closure coefficientis, $\beta_2 = 0.0828$ and $\sigma_{\omega, 2} = 0.856$.

*Low-Reynolds formulation*:

\begin{equation}
\gamma = F_1 \gamma_1 \frac{1}{\alpha^*} \frac{1/9 + Re_{\tau}/Re_{\omega}}{1.0 + Re_{\tau}/Re_{\omega}} + (1 - F_1) \gamma_2 \,,
\end{equation}

where:

- $Re_{\tau} = \frac{\rho k}{\mu \omega}$ is the non-equilibrium friction Reynolds number,
- $Re_{\omega} = 2.95$ is a closure parameter for the low Reynolds limit,
- $\alpha^*$ is a closure function, which is defined as explained in [kOmegaSSTViscosityAux](kOmegaSSTViscosityAux.md).


The destruction of the turbulent kinetic energy specific dissipation rate is modeled as follows:

\begin{equation}
\epsilon_{\omega} = \rho \beta f_{\beta} \omega^2 \,,
\end{equation}

where:

- $\rho$ is the density,
- $\beta = F_1 \beta_1 + (1 - F_1) \beta_2$ is a closure coefficient, with $\beta_1 = 0.075$ and $\beta_2 = 0.0828$,
- $f_{\beta}$ is a closure function, which formulation depends on wether the standard or vortex stretching modifications are used for the model (the vortex stretching modification of the model is activated with the [!param](/FVKernels/INSFVTKESDSourceSink/vortex_stretching_modficiation) parameter).

*Standard formulation*:

\begin{equation}
f_{\beta} = 1.0 \,,
\end{equation}

*Free-shear formulation*:

\begin{equation}
f_{\beta} = \frac{1 + 70 \chi_{\omega}}{1 + 80 \chi_{\omega}} \,,
\end{equation}

where:

- $\chi_{\omega} = \frac{(\mathbf{W} \cdot \mathbf{W}) : \mathbf{S}}{(\beta^* \omega^3)}$ with the shear strain tensor defined as $\mathbf{S} = \frac{1}{2} [\nabla \vec{u} + (\nabla \vec{u})^T]$, the rotation rate tensor defined as $\mathbf{W} = \frac{1}{2} [\nabla \vec{u} - (\nabla \vec{u})^T]$, and $\beta^*$ defined as explained in [INSFVTKESourceSink](INSFVTKESourceSink.md).

Finally, the cross diffusion term is modeled as follows:

\begin{equation}
CD_{\omega} = 2 \rho (1 - F_1) \sigma_{\omega, 2} \frac{1}{\omega} \nabla k \cdot \nabla w - \delta CD_{\omega, y}\,,
\end{equation}

where:

- $\sigma_{\omega, 2} = 0.856$ is a closure coefficient,
- $\delta$ is an activation limiter function which is $0$ for the standard formulation of the model and $1$ when the free-shear modification is activated via the [!param](/FVKernels/INSFVTKESDSourceSink/free_shear_modification) parameter,
- $CD_{\omega, y}$ is a cross diffusion limiter, which is formulated as follows:

\begin{equation}
CD_{\omega, y} = \sigma_d \frac{\rho}{\omega} \frac{\partial k}{\partial y} \frac{\partial \omega}{\partial y}\,,
\end{equation}

where:
- $\sigma_d$ is a conditional activation function that is $\sigma_d = 1/8$ when $\frac{\partial k}{\partial y} \frac{\partial \omega}{\partial y} > 0$ and $\sigma_d = 0$ otherwise,

The cross-diffusion limiter acts only in the distance perpendicular to the wall $y$.
For this purpose, the normal to the nearest wall needs to be defined via the [!param](/FVKernels/INSFVTKESDSourceSink/wall_normal_unit_vectors) parameter.
We recomend using the [WallNormalUnitVectorAux](WallNormalUnitVectorAux.md) AuxKernel for this purpose.


## Wall formulation:

All cells in contact with a boundary identified in the [!param](/FVKernels/INSFVTKESDSourceSink/walls) list are applied a different
treatment in which the equilibrium value for the $\omega = \omega{eq}$ is set.
A blended wall function is used for all $y^+$.
This blending is defined via a blending parameter $\Gamma$ defined as follows:

\begin{equation}
\Gamma = e^{-\frac{Re_k}{11}} \,,
\end{equation}

where:

- $Re_k = \frac{\rho \sqrt{k} y_p}{\mu}$ is the non-equilibrium friction Reynolds number, with $k$ being the turbulent kinetic energy at the near-wall cells and $y_p$ being the ditance to the nearest wall.

Using this blending function, the destruction near the wall is formulated as follows:

\begin{equation}
\epsilon = \Gamma \epsilon_{visc} + (1 - \Gamma) \epsilon_{log}\,.
\end{equation}

where $\epsilon_{visc}$ and $\epsilon_{log}$ are the destruction in the viscous andlogarithmic layer, respectively, which are defined as follows:

\begin{equation}
\epsilon_{visc} = \frac{6 \mu}{\rho \beta_1 y_p^2} \,.
\end{equation}

where:

- $\beta_1 = 0.075$ is a closure parameter for the near-wall formulation.

\begin{equation}
\epsilon_{log} = \frac{\sqrt{k}}{C_{\mu}^{0.25} \kappa y_p} \,.
\end{equation}

where:

- $C_{\mu} = 0.09$ is a closure parameter.


!alert note
When using wall functions, since the equilibrium value for $\omega$ is set in the cells near the wall, the user is recommended to deactivate advection and diffusion for those near wall cells.

!alert note
When the wall treatment is specified in this kernel, any boundary condition for $\omega$ will be ignored.
In other words, there is no need to impose boundary conditions for $\omega$ when the wall treatment
is specified in his kernel.

!alert note
We recomend to deactivate the near-wall treatment when performing enhanced wall treatment.

!syntax parameters /FVKernels/INSFVTKESDSourceSink

!syntax inputs /FVKernels/INSFVTKESDSourceSink

!syntax children /FVKernels/INSFVTKESDSourceSink
