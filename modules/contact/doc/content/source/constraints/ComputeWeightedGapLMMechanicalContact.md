# ComputeWeightedGapLMMechanicalContact

The Karush-Kuhn-Tucker conditions of mechanical contact are:

\begin{equation}
\begin{aligned}
g &\ge 0\\
\lambda &\ge 0\\
g\lambda &= 0
\end{aligned}
\end{equation}

where $g$ is the gap and $\lambda$ is the contact pressure, a Lagrange multiplier
variable living on the secondary face. Per [!citep](wohlmuth2011variationally)
and [!citep](popp2014dual), the variationally consistent, discretized version of
the KKT conditions are:

\begin{equation}
\begin{aligned}
(\tilde{g}_n)_j &\ge 0\\
(\lambda_n)_j &\ge 0\\
(\tilde{g}_n)_j (\lambda_n)_j &= 0
\end{aligned}
\end{equation}

where $n$ indicates the normal direction, $j$ denotes the j'th secondary contact
interface node, and $(\tilde{g}_n)_j$ is the discrete weighted gap, computed by:

\begin{equation}
(\tilde{g}_n)_j = \int_{\gamma_c^{(1)}} \Phi_j g_{n,h} dA
\end{equation}

where $\gamma_c^{(1)}$ denotes the secondary contact interface, $\Phi_j$ is the
j'th lagrange multiplier test function, and $g_{n,h}$ is the discretized version
of the gap function.

The `ComputeWeightedGapLMMechanicalContact` object computes the weighted gap and
applies the KKT conditions. The KKT conditions
are enforced using a nonlinear complementarity problem (NCP) function, in this case the most
simple such function, $min(c(\tilde{g}_n)_j, (\lambda_n)_j)$, where $c$ (implemented with the input
parameter `c`) is used to balance the size of the gap
and the normal contact pressure. If the contact pressure is of order 10000, and the
gap is of order .01, then `c` should be set to 1e6 in order to bring
components of the NCP function onto the same level and achieve optimal
convergence in the non-linear solve.

Mechanical mortar contact uses normalized, weighted secondary nodal normals to evaluate the
weighted gap. Supported quasistatic local-basis contact includes the displacement derivatives of
the nodal normal directions in Jacobian evaluations. This behavior applies to the `mortar` and
non-augmented `mortar_penalty` formulations with `frictionless` or `coulomb` contact and
[!param](/Constraints/ComputeWeightedGapLMMechanicalContact/interpolate_normals) set to `false`.
The normal, tangent, weighted-gap, tangential-velocity, traction, and residual values at a fixed
solution state remain unchanged. Residual evaluations contain no derivatives of the normal or
tangent directions. The Jacobian includes the normal-direction term in the weighted-gap derivative,

!equation
\delta (\boldsymbol{G}_A \cdot \boldsymbol{n}_A)
= \delta\boldsymbol{G}_A \cdot \boldsymbol{n}_A
+ \boldsymbol{G}_A \cdot \delta\boldsymbol{n}_A,

and the corresponding direction derivatives in normal traction and tangential velocity,

!equation
\delta(\lambda_A\boldsymbol{n}_A)
= \delta\lambda_A\boldsymbol{n}_A + \lambda_A\delta\boldsymbol{n}_A,
\qquad
\delta(\boldsymbol{v}_A\cdot\boldsymbol{t}_{A\alpha})
= \delta\boldsymbol{v}_A\cdot\boldsymbol{t}_{A\alpha}
+ \boldsymbol{v}_A\cdot\delta\boldsymbol{t}_{A\alpha}.

The corresponding weighted-gap or weighted-velocity user objects and primal normal-traction,
tangential-friction, and penalty contact constraints use the same linearized directions.
Mortar test functions, coordinate factors, and dual or Petrov-Galerkin basis choices are unchanged;
their weighted contributions are already contained in \(\boldsymbol{G}_A\).

Dynamic mortar through [ContactAction](/ContactAction.md) uses frozen normal and tangent directions
in its Jacobian. Augmented-Lagrangian penalty, Cartesian-LM, cohesive-zone, and nonmortar contact
also use frozen directions. Quadrature-point normal interpolation is not supported for this
local-basis contact path. The displacement variables must be nodal nonlinear variables in the
system assembled by the contact objects.

Only the averaged secondary nodal-normal field and its derived Householder tangents are
differentiated. Mortar segment topology, primary-secondary projections and parent reference
coordinates, mortar-segment `JxW`, moving overlap boundaries, and active-set decisions remain
fixed. The contact Jacobian therefore includes additional nodal-direction derivative terms, but
this does not guarantee quadratic Newton convergence.

The normal derivatives span the incident-face star of each secondary node, so the required
AD derivative capacity can increase relative to a fixed normal direction. The requirement depends
on the secondary face order, node valence, displacement variables, and system degree-of-freedom
layout. The configured sparse AD container must be large enough for the largest scalar normal or
tangent component in the target mesh; no single derivative size is appropriate for every problem.
If the capacity is insufficient, reconfigure with a larger `--with-derivative-size` value and
rebuild the MOOSE libraries and application consistently.

Moving distributed contact can introduce matrix couplings that were not present during initial
sparsity preallocation. For these problems, set
[!param](/Problem/FEProblem/use_hash_table_matrix_assembly) to `true`. Relationship managers make
the required remote interface elements available, while hash-table assembly accommodates changing
couplings between degrees of freedom owned by the same process.

!syntax description /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax parameters /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax inputs /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax children /Constraints/ComputeWeightedGapLMMechanicalContact
