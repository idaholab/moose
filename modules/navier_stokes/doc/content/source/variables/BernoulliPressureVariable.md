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

The decision to use downstream information to compute the upstream pressure value
is based on eigenvalue reasoning for the subsonic Euler equations. For the
subsonic Euler equations, mesh dimension + 1 upstream explicit/physical (as
opposed to implicit) boundary conditions are required while one downstream
explicit/physical boundary condition is required
[!citep](novak2018pronghorn). In practice this often corresponds to mesh
dimension explicit inlet boundary conditions related to velocity, one explicit
inlet boundary condition related to temperature, and one explicit outlet
boundary condition related to pressure. Thus physics-based discretizations of
Euler flows typically (at least partially) upwinds information being advected by
the flow field. However, given the propagation of pressure information upstream
from the explicit outlet pressure boundary condition, we believe it reasonable
to do the same for computing the Bernoulli pressure jump at porosity
discontinuities.

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
