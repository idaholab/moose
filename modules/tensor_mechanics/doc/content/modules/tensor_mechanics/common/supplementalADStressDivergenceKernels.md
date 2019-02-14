The stress divergence kernel handles the calculation of the residual,
$\mathbb{R}$, from the governing equation and the calculation of the Jacobian
using forward mode automatic differentiation. From the strong form of the
governing equation for mechanics, neglecting body forces,

\begin{equation}
\nabla \cdot \sigma = 0
\end{equation}

the weak form, using Galerkin's method and the Gauss divergence theorem, becomes

\begin{equation}
\int_S n \cdot (\sigma \psi) dS - \int_V \sigma \cdot \nabla \psi dV = 0
\end{equation}

in which $\psi$ is the test function. The second term of the weak form equation
is the residual contribution calculated by the stress divergence kernel.

The `use_displaced_mesh` parameter must be set correcting to ensure consistency
in the equilibrium equation: if the stress is calculated with respect to the
deformed mesh, the test function gradients must also be calculated with respect
to the deformed mesh.

!alert note title=Automatic Differentiation Materials
The computation of a correct Jacobian contribution requires the use of
compatible automatic differentiation materials (`[ADMaterials]`).
