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
applies the KKT conditions. The KKT conditions are enforced using a nonlinear
complementarity problem (NCP) function. By default this is the simple min
function, $\min(c(\tilde{g}_n)_j, (\lambda_n)_j)$, where $c$ (implemented with the
input parameter [!param](/Constraints/ComputeWeightedGapLMMechanicalContact/c)) is
used to balance the size of the gap and the normal contact pressure. If the
contact pressure is of order 10000, and the gap is of order .01, then
[!param](/Constraints/ComputeWeightedGapLMMechanicalContact/c) should be set to
1e6 in order to bring components of the NCP function onto the same level and
achieve optimal convergence in the non-linear solve.

The [!param](/Constraints/ComputeWeightedGapLMMechanicalContact/normal_ncp_function)
parameter selects the NCP function used for this normal constraint. The default
`MIN` option preserves the historical residual. `SMOOTH_MIN` replaces the corner
of the min function with a differentiable approximation, while
`FISCHER_BURMEISTER` and `SMOOTH_FISCHER_BURMEISTER` use the corresponding
Fischer-Burmeister NCP functions. The exact Fischer-Burmeister option remains
nonsmooth at the origin, where the implementation uses a finite generalized
derivative. The smooth options require a positive
[!param](/Constraints/ComputeWeightedGapLMMechanicalContact/normal_ncp_smoothing_width),
whose value is measured on the same scale as $(\lambda_n)_j$ and
$c_\mathrm{eff}(\tilde{g}_n)_j$, where $c_\mathrm{eff}$ is the effective value of
[!param](/Constraints/ComputeWeightedGapLMMechanicalContact/c) after any
[!param](/Constraints/ComputeWeightedGapLMMechanicalContact/normalize_c)
normalization.

The smoothing width is not normalized internally. After choosing
[!param](/Constraints/ComputeWeightedGapLMMechanicalContact/c), choose a
dimensioned width from one of the two arguments of the normal NCP function, for
example $\epsilon = \alpha |(\lambda_n)_j|$ using a representative active-contact
normal Lagrange multiplier, or
$\epsilon = \alpha |c_\mathrm{eff}(\tilde{g}_n)_j|$ using a representative
effective scaled weighted gap. Typical trial factors are roughly
$\alpha = 10^{-8}$ to $10^{-4}$. Smaller values approach the exact nonsmooth
complementarity function more closely; larger values create a wider
differentiable transition but can also round the active-set transition enough to
perturb the contact pressure and gap response.

!syntax description /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax parameters /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax inputs /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax children /Constraints/ComputeWeightedGapLMMechanicalContact
