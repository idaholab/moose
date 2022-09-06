# ComputeWeightedGapCartesianLMMechanicalContact

The Karush-Kuhn-Tucker conditions of mechanical contact are:

\begin{equation}
\begin{aligned}
g &\ge 0\\
\lambda &\ge 0\\
g\lambda &= 0
\end{aligned}
\end{equation}

where $g$ is the gap and $\lambda$ is the contact pressure, a Lagrange multipler
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

The KKT conditions are enforced using a nonlinear complimentarity problem (NCP) function, in this case the most
simple such function, $min(c(\tilde{g}_n)_j, (\lambda_n)_j)$, where $c$ (implemented with the input
parameter `c`) is used to balance the size of the gap
and the normal contact pressure. If the contact pressure is of order 10000, and the
gap is of order .01, then `c` should be set to 1e6 in order to bring
components of the NCP function onto the same level and achieve optimal
convergence in the non-linear solve.

The `ComputeWeightedGapCartesianLMMechanicalContact` object computes the weighted gap and
applies the KKT conditions using Lagrange multipliers defined in a global Cartesian reference frame.
As a consequence, the number of contact constraints at each node will be two, in two-dimensional problems,
and three, in three-dimensional problems. The normal contact pressure is obtained by projecting the Lagrange
multiplier vector along the normal vector computed from the mortar generation objects. The result is a normal
contact constraint, which, in general, will be a function of all (two or three) Cartesian Lagrange multipliers.
This methodology only constrains one degree of freedom. The other degree(s) of freedom are constrained by
enforcing that tangential tractions are identically zero. Note that, if friction with Cartesian Lagrange multipliers
is chosen via [ComputeFrictionalForceCartesianLMMechanicalContact](/ComputeFrictionalForceCartesianLMMechanicalContact.md),
those remaining nodal degrees of freedom are constraint using Coulomb constraints within a semi-smooth Newton approach. Usage of
Cartesian Lagrange multipliers is recommended when condensing Lagrange multipliers via the variable condensation preconditioner
(VCP) [VariableCondensationPreconditioner](/VariableCondensationPreconditioner.md).

The user can also employ locally oriented Lagrange multipliers [ComputeWeightedGapLMMechanicalContact](/ComputeWeightedGapLMMechanicalContact.md),
which minimizes the number of contact constraints for frictionless problems.

!syntax description /Constraints/ComputeWeightedGapCartesianLMMechanicalContact

!syntax parameters /Constraints/ComputeWeightedGapCartesianLMMechanicalContact

!syntax inputs /Constraints/ComputeWeightedGapCartesianLMMechanicalContact

!syntax children /Constraints/ComputeWeightedGapCartesianLMMechanicalContact
