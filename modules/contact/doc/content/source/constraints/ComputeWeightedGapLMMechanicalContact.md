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
weighted gap. For supported quasistatic local-basis contact, Jacobian-bearing evaluations include
the displacement derivatives of the nodal normal directions by default. This behavior applies to
the `mortar` and non-augmented `mortar_penalty` formulations with `frictionless` or `coulomb`
contact and
[!param](/Constraints/ComputeWeightedGapLMMechanicalContact/interpolate_normals) set to `false`.
The normal, tangent, weighted-gap, tangential-velocity, traction, and residual values at a fixed
solution state remain unchanged. Residual-only mode uses the stored normal and tangent values
directly and does not populate the AD geometry cache. The Jacobian includes the normal-direction
term in the weighted-gap derivative,

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
The existing mortar test functions, coordinate factors, and dual or Petrov-Galerkin basis choices
are unchanged; their weighted contributions are already contained in \(\boldsymbol{G}_A\).

Dynamic mortar contact, augmented-Lagrangian penalty contact, Cartesian-LM contact, cohesive-zone
models (CZM), and nonmortar formulations retain their existing Jacobian behavior. Quadrature-point
normal interpolation is not part of this local-basis linearization, and the mortar formulations
already exclude the `glued` model. The displacement variables for supported contact must be nodal
nonlinear variables in the system assembled by the contact objects.

Only the averaged secondary nodal-normal field and its derived Householder tangents are linearized.
The Householder tangent frame has an unavoidable nondifferentiable pole at
`normal = (-1, 0, 0)`. MOOSE preserves its historical near-pole branch and assigns zero tangent
derivatives while that piecewise constant branch is active.
Mortar segment topology, primary-secondary projections and parent reference coordinates,
mortar-segment `JxW` factors and moving overlap boundaries, and active-set decisions remain frozen;
their derivatives are omitted. Consequently, the additional direction derivatives improve the
contact Jacobian but do not guarantee quadratic Newton convergence. The one-ring normal-derivative
stencil can enlarge the matrix sparsity pattern. Its closure includes primary elements in the mortar
overlap known when the sparsity pattern is built. As with the existing mortar coupling graph,
sufficiently large sliding that creates a previously unknown primary-secondary overlap can require
PETSc to extend the matrix pattern during the solve.
The mortar-penalty Coulomb anti-ping-pong limiter also retains its established frozen scalar
traction-rescaling coefficient; derivatives of the AD slip vector are included, while derivatives
of the normal pressure and slip norm inside that limiter coefficient are omitted.

The normal derivative stencil spans the incident-face star of each secondary node, so the required
AD derivative capacity can increase relative to a fixed normal direction. The requirement depends
on the secondary face order, node valence, displacement variables, and system degree-of-freedom
layout. The configured sparse AD container must be large enough for the largest scalar normal or
tangent component in the target mesh; no single derivative size is appropriate for every problem.
If the capacity is insufficient, reconfigure with a larger `--with-derivative-size` value and
rebuild the MOOSE libraries and application consistently.

The coordinate-sensitivity implementation supports `EDGE2`, `EDGE3`, `TRI3`, `TRI6`, `TRI7`,
`QUAD4`, `QUAD8`, and `QUAD9` secondary mortar elements. A supported Jacobian evaluation reports an
error on another secondary element type.

!syntax description /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax parameters /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax inputs /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax children /Constraints/ComputeWeightedGapLMMechanicalContact
