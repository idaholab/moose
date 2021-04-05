# ApplyPenetrationConstraintLMMechanicalContact

The `ApplyPenetrationConstraintLMMechanicalContact` class is used to ensure that the
Karush-Kuhn-Tucker conditions of mechanical contact are satisfied:

\begin{equation}
\begin{aligned}
g &\ge 0\\
\lambda &\ge 0\\
g\lambda &= 0
\end{aligned}
\end{equation}

where $g$ is a weighted gap and $\lambda$ is the contact pressure, a Lagrange multipler
variable living on the secondary face. This object must always be used in
conjunction with [ComputeWeightedGapLMMechanicalContact.md] which computes the
weighted gap. The constraint enforcement with the weighted gap is variationally
consistent as derived in [!citep](wohlmuth2011variationally). The KKT conditions
are enforced using a nonlinear complimentarity problem (NCP) function, in this case the most
simple such function, $min(cg, \lambda$), where $c$ (implemented with the input
parameter `c`) is used to balance the size of the gap
and the normal contact pressure. If the contact pressure is of order 10000, and the
gap is of order .01, then `c` should be set to 1e6 in order to bring
components of the NCP function onto the same level and achieve optimal
convergence in the non-linear solve.

!syntax description /Constraints/ApplyPenetrationConstraintLMMechanicalContact

!syntax parameters /Constraints/ApplyPenetrationConstraintLMMechanicalContact

!syntax inputs /Constraints/ApplyPenetrationConstraintLMMechanicalContact

!syntax children /Constraints/ApplyPenetrationConstraintLMMechanicalContact
