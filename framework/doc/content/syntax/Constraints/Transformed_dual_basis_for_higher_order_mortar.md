# Transformed dual basis for higher-order mortar

## Overview

Dual (biorthogonal) Lagrange multiplier bases are attractive for mortar contact because they make
the mortar coupling matrix on the secondary side diagonal, which enables local condensation of the
multiplier degrees of freedom. MOOSE builds the dual basis $\Phi$ so that it is biorthogonal to the
standard trace basis $N$,

\begin{equation}
\int_{\gamma} \Phi_j \, N_k \, \mathrm{d}\gamma = \delta_{jk} \, d_k, \qquad
d_k = \int_{\gamma} N_k \, \mathrm{d}\gamma ,
\end{equation}

where $\gamma$ is the secondary contact face and $d_k$ is the diagonal entry associated with node
$k$. For first-order faces (e.g. QUAD4, TRI3) every $d_k$ is strictly positive and this construction
is well posed.

For *quadratic* Lagrange faces the construction breaks down because the raw diagonal is no longer
positive: on a QUAD8 face the corner nodes have $d_k = -1/3$ and on a TRI6 face the vertex nodes
have $d_k = 0$. The per-node physical-gap normalization used by mortar mechanical contact divides
the weighted gap by $\int_{\gamma} \Phi_j\,\mathrm{d}\gamma = d_j$, so a zero or negative diagonal
makes the nodal contact logic ill posed. This is why standard dual mortar contact in MOOSE is
restricted to first-order secondary faces.

[!cite](popp2012dual) resolve this with a *locally quadratic transformed trace basis*
$\tilde{N} = T\,N$, in which each vertex node absorbs a fixed fraction $\alpha$ of its adjacent
mid-edge nodes and each mid-edge node is scaled accordingly,

\begin{equation}
\tilde{N}_v = N_v + \alpha \sum_{e \,\in\, \mathrm{adj}(v)} N_e , \qquad
\tilde{N}_m = (1 - 2\alpha)\, N_m ,
\end{equation}

where $v$ ranges over vertex nodes, $m$ over mid-edge nodes, and $\mathrm{adj}(v)$ is the set of
mid-edge nodes adjacent to vertex $v$. The transform preserves the partition of unity
($\sum_k \tilde{N}_k = \sum_k N_k = 1$), so patch-test consistency ($\sum_j \tilde{\Phi}_j = 1$) is
retained. With the value $\alpha = 1/5$ (used for both TRI6 and QUAD8), the transformed diagonal
$\tilde{d}_k = \int_{\gamma} \tilde{N}_k\,\mathrm{d}\gamma$ is strictly positive:

| Face  | standard $d$ (vertex / mid) | transformed $\tilde{d}$ at $\alpha = 1/5$ (vertex / mid) |
| ----- | --------------------------- | ------------------------------------------------------- |
| QUAD8 | $-1/3$ / $+4/3$             | $1/5$ / $4/5$                                            |
| TRI6  | $0$ / $+1/6$               | $1/15$ / $1/10$                                          |

The dual basis is then made biorthogonal to the *transformed* trace basis,
$\int_{\gamma} \tilde{\Phi}_j\,\tilde{N}_k\,\mathrm{d}\gamma = \delta_{jk}\,\tilde{d}_j$, which
restores a well-posed, positive-diagonal dual basis on quadratic faces and therefore enables dual
mortar contact on TET10 and HEX20 meshes. Only the secondary trace basis used for
biorthogonalization is transformed; the standard quadratic geometry and displacement interpolation
is unchanged, and the dual basis is still expressed in the standard basis $N$ so consistency is
preserved. When $T = I$ (i.e. on lower-order or QUAD9 faces) the original construction is recovered
exactly.

## Implementation and Usage Details

The transform is applied inside the dual-coefficient computation and is triggered only for TRI6 and
QUAD8 lower-dimensional secondary faces; all other face types (including QUAD9 and first-order
faces) fall through to the standard dual construction unchanged. The transform is applied
automatically whenever a dual basis is active on a TRI6 or QUAD8 secondary face; there is no user
option to enable or disable it. First-order dual mortar results are bit-identical because the
transform never fires on first-order faces.

The transform is not combined with the Petrov-Galerkin dual mortar approach, and the two are mutually
exclusive on quadratic secondary faces. The transform repairs the *dual trial* basis used to
interpolate the multiplier, but Petrov-Galerkin weights the multiplier with the *standard test* basis
$N$ [!cite](popp2013improved), whose per-node normalization $\int_\gamma N_k\,\mathrm{d}\gamma$ is the
same non-positive quantity ($-1/3$ at a QUAD8 corner, $0$ at a TRI6 vertex) that motivates the
transform in the first place. Because the transform acts on the trial basis rather than the test
basis, it cannot restore Petrov-Galerkin weighted-gap positivity on quadratic faces; that requires a
separate treatment which is out of scope here. Requesting `use_petrov_galerkin = true` for dual mortar
on a QUAD8 or TRI6 secondary face is therefore rejected with an error rather than silently solving
with the ill-posed standard dual.

In the [Contact](Contact/index.md) action, request the dual basis with `use_dual = true`; the
transform is then applied automatically on quadratic secondary faces:

```
[Contact]
  [mortar]
    primary = 'primary_surface'
    secondary = 'secondary_surface'
    formulation = mortar
    model = frictionless
    c_normal = 1e4
    use_dual = true
  []
[]
```

In a `Constraints`-based input, request the dual basis on the second-order Lagrange multiplier
variable itself; the transform is applied automatically:

```
[Variables]
  [lm]
    block = 'secondary_lower'
    order = SECOND
    use_dual = true
  []
[]
```

The transform has no effect on lower-order or QUAD9 faces, and it composes with edge dropping
(`correct_edge_dropping = true`).

!alert note title=Reported weighted gap becomes a neighbor-weighted blend
Because the transformed trace basis mixes each vertex with its adjacent mid-edge nodes, the
*reported* nodal weighted gap on a transformed face is a $T$-weighted blend of the neighboring
nodal gaps rather than a pointwise nodal value. The *enforced* contact conditions remain correct
(both the multiplier and the transformed weighted gap are native quantities in the transformed
space); only the diagnostic per-node gap value carries this reinterpretation.

!alert warning title=Interaction with the VariableCondensationPreconditioner
The transformed dual basis makes the coupling matrix $D$ non-diagonal (biorthogonality holds
against the transformed trace basis, not the standard one). When condensing the multiplier with the
[VariableCondensationPreconditioner.md], use the exact inverse path
(`is_lm_coupling_diagonal = false`). With `is_lm_coupling_diagonal = true` only the diagonal of $D$
is inverted, so the preconditioner is approximate and convergence may degrade; MOOSE emits a
warning in that case. The converged solution is unaffected because the Krylov solver still applies
the true operator.

## References

!bibtex bibliography
