# MassFluxPenaltyIPHDG

Unlike the perturbation in [MassFluxPenalty.md], this class does not introduce coupling between interior degress of freedom on adjacent elements. Instead it imposes the face term

\begin{equation}
\label{eq:perturbation}
\langle u - \bar{u}, v - \bar{v}\rangle
\end{equation}

where $u$ is the primal finite element solution (defined on element interiors), $\bar{u}$ is the facet solution, $v$ are the primal test functions, and $\bar{v}$ are the facet test functions. The augmented form of the IP-HDG discretized Navier-Stokes equations is algebraically equivalent to

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
and $\mathcal{R}(B^T) = \mathcal{R}(J)$. [eq:perturbation] meets these requirements.

!syntax parameters /HDGKernels/MassFluxPenaltyIPHDG

!syntax inputs /HDGKernels/MassFluxPenaltyIPHDG

!syntax children /HDGKernels/MassFluxPenaltyIPHDG
