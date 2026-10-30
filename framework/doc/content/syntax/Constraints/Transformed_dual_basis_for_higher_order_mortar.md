# Transformed dual basis for higher-order mortar

A dual (biorthogonal) Lagrange multiplier basis $\Phi$ satisfies

\begin{equation}
\int_{\gamma} \Phi_j \, N_k \, \mathrm{d}\gamma = \delta_{jk} \, d_k, \qquad
d_k = \int_{\gamma} N_k \, \mathrm{d}\gamma ,
\end{equation}

where $N$ is the standard basis on the secondary face $\gamma$. Mortar mechanical contact divides
the weighted gap of node $j$ by $d_j$, so every $d_j$ must be positive. On QUAD8 and TRI6 faces,
the faces of HEX20 and TET10 elements, it is not: $d_k = -1/3$ at QUAD8 corners and $d_k = 0$ at
TRI6 vertices, where the dual shape functions vanish ([!cite](popp2012dual), Eqs. (4.3) and (4.4)).

libMesh therefore builds the dual basis of a SECOND order Lagrange multiplier on these faces against
the locally quadratic transformed basis of [!cite](popp2012dual), Sec. 4.4.1,

\begin{equation}
\tilde{N}_v = N_v + \alpha \sum_{m \,\in\, \mathrm{adj}(v)} N_m , \qquad
\tilde{N}_m = (1 - 2\alpha)\, N_m , \qquad \alpha = 1/5,
\end{equation}

where $v$ is a vertex node and $\mathrm{adj}(v)$ are its adjacent mid-edge nodes $m$. The
transform keeps the partition of unity and makes all weights
$\tilde{d}_k = \int_{\gamma} \tilde{N}_k\,\mathrm{d}\gamma$ positive:

| Face  | $d$ (vertex / mid-edge) | $\tilde{d}$ (vertex / mid-edge) |
| ----- | ----------------------- | ------------------------------- |
| QUAD8 | $-1/3$ / $4/3$          | $1/5$ / $4/5$                   |
| TRI6  | $0$ / $1/6$             | $1/15$ / $1/10$                 |

The transform is applied automatically whenever such a multiplier sets `use_dual = true`, which is
the default for mortar in the [Contact](Contact/index.md optional=True) action. Other faces,
including QUAD9, and FIRST order multipliers keep the standard dual basis. Because biorthogonality
holds against $\tilde{N}$, the coupling matrix $\int_{\gamma} \Phi_j N_k\,\mathrm{d}\gamma$ is not
diagonal on transformed faces.

The Petrov-Galerkin approach ([Petrov-Galerkin_approach_for_Lagrange_multipliers.md]) weights the
gap with the standard basis $N$, whose nodal integrals $d_k$ are not positive on these faces. MOOSE
therefore reports an error when `use_petrov_galerkin = true` is combined with a SECOND order dual
multiplier on QUAD8 or TRI6 secondary faces, unless the gap is weighted with the piecewise-linear
basis described there.

## References

!bibtex bibliography
