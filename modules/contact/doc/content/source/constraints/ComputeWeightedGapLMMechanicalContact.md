# ComputeWeightedGapLMMechanicalContact

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

The `ComputeWeightedGapLMMechanicalContact` object computes the weighted gap and
applies the KKT conditions. The KKT conditions
are enforced using a nonlinear complimentarity problem (NCP) function, in this case the most
simple such function, $min(c(\tilde{g}_n)_j, (\lambda_n)_j)$, where $c$ (implemented with the input
parameter `c`) is used to balance the size of the gap
and the normal contact pressure. If the contact pressure is of order 10000, and the
gap is of order .01, then `c` should be set to 1e6 in order to bring
components of the NCP function onto the same level and achieve optimal
convergence in the non-linear solve.

!syntax description /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax parameters /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax inputs /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax children /Constraints/ComputeWeightedGapLMMechanicalContact
