# BernoulliPressureVariable

This variable type is specific to the porous media incompressible Navier Stokes
equations. When used instead of a typical finite volume variable, faces for
which the neighboring elements have different porosity values will be treated as
either Dirichlet or extrapolated boundary faces. When this variable is queried for a face value
on the downwind side of the face, only downwind information is used to
extrapolate and reconstruct the downwind side face value. The upwind side face
value is computed using the reconstructed downwind face pressure value and the
Bernoulli equation:

\begin{equation}
p_1 + \frac{1}{2}\rho_1\vec{v}_1^2 = p_2 + \frac{1}{2}\rho_2\vec{v}_2^2
\end{equation}

where $p$ is the pressure, $\rho$ is the density, and $\vec{v}$ is the
interstitial velocity (not the superficial velocity). Bernoulli's equation
typically contains gravitational terms; however, we have omitted them under the
assumption that $\rho_1 = \rho_2$ which should be true when density does not
depend on pressure (the incompressible or "weakly" compressible case).

Furthermore, the user can select sidesets in the domain to assign additional
irreversible pressure drops. For this, the selected sidesets should be defined
using parameters [!param](/Variables/BernoulliPressureVariable/pressure_drop_sidesets)
and [!param](/Variables/BernoulliPressureVariable/pressure_drop_form_factors).
The irreversible pressure drop can be expressed as:

\begin{equation}
\frac{1}{2}\kappa\rho\vec{v}^2,
\end{equation}

where $\kappa$ is the form loss coefficient. For contractions, the velocity and the density
on the upwind side are taken whereas for expansions the downwind side values are used.

!alert! note title=Behavior with parallel execution

In certain cases multiple porosity jump faces may be connected by cells in a
chain. For example at corners or when porous medium zones are one-cell wide. In such
scenarios, the two-term expansion for the determination of the
face pressure on the downstream side requires a considerably extended stencil which may not be accommodated by the
number of ghosting layers set in the kernels. For this reason, the default value of
[!param](/Variables/BernoulliPressureVariable/allow_two_term_expansion_on_bernoulli_faces)
is `false`. If the user wants to enable two-term expansion for the pressure
computation on the porosity jump faces, special attention should be paid to
moving the porosity jump faces sufficiently far from each other (at least two layers
if skewness correction is disabled and three if it is enabled) or adding
additional layers of ghosted elements (which can potentially increase local computational and memory costs)
using the following `FVKernel` parameter [!param](/FVKernels/FVDiffusion/ghost_layers).

!alert-end!

!syntax parameters /Variables/BernoulliPressureVariable

!syntax inputs /Variables/BernoulliPressureVariable

!syntax children /Variables/BernoulliPressureVariable
