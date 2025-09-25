# MassFluxPenaltyIPHDG

Unlike the perturbation in [MassFluxPenalty.md], this class does not introduce coupling between
interior degress of freedom on adjacent elements. Instead it imposes on the momentum component
equations the element face term

\begin{equation}
\label{eq:perturbation}
\langle h_F^{-1} \left(u - \bar{u}\right)\cdot\hat{n}, \left(v - \bar{v}\right)\cdot\hat{n}\rangle
\end{equation}

where $h_F$ is the characteristic size of the facet (equivalent to $h_K$ for a shape-regular mesh), $u$ is the primal finite element solution (defined on element interiors), $\bar{u}$ is the facet solution, $v$ are the primal test functions, $\bar{v}$ are the facet test functions, and $\hat{n}$ is the normal vector. The augmented form of the IP-HDG discretized Navier-Stokes equations is algebraically equivalent to

\begin{equation*}
  \mathbb{A} =
  \begin{bmatrix}
    A + \gamma J & B^T \\
    B & \mathbf{0}
  \end{bmatrix}.
\end{equation*}

where $A$ is a nonsingular $n \times n$ matrix, $J \ne 0$ is a
symmetric, singular $n \times n$ matrix, $B$ is an $m \times n$
matrix ($m < n$), and $\gamma>0$ is a scalar
constant. Requirements for the singular perturbation are that $\mathcal{N}(J)\subseteq \mathcal{N}(B)$
and $\mathcal{R}(B^T) = \mathcal{R}(J)$. [eq:perturbation], corresponding to
the singular perturbation $J$, meets these requirements.

!syntax parameters /HDGKernels/MassFluxPenaltyIPHDG

!syntax inputs /HDGKernels/MassFluxPenaltyIPHDG

!syntax children /HDGKernels/MassFluxPenaltyIPHDG
