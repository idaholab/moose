# NormalNodalLMMechanicalContact

The `NormalNodalLMMechanicalContact` class is used to ensure that the
Karush-Kuhn-Tucker conditions of mechanical contact are satisfied:

\begin{equation}
\begin{aligned}
g &\ge 0\\
\lambda &\ge 0\\
g\lambda &= 0
\end{aligned}
\end{equation}

where $g$ is the gap and $\lambda$ is the contact pressure, a Lagrange multipler
variable living on the secondary face. This object enforces constraints nodally so
that zero penetration of nodes is observed into the primary face.

The `ncp_function_type` parameter specifies the type of nonlinear
complimentarity problem (NCP) function to use. The options are either `min`, which is just the
min function, or `fb` which represents the Fischer-Burmeister function. In our
experience, the min function achieves better convergence in the
non-linear solve. The `c` parameter is used to balance the size of the gap
and the normal contact pressure. If the contact pressure is of order 10000, and the
gap is of order .01, then `c` should be set to 1e6 in order to bring
components of the NCP function onto the same level and achieve optimal
convergence in the non-linear solve.

!syntax description /Constraints/NormalNodalLMMechanicalContact

!syntax parameters /Constraints/NormalNodalLMMechanicalContact

!syntax inputs /Constraints/NormalNodalLMMechanicalContact

!syntax children /Constraints/NormalNodalLMMechanicalContact
