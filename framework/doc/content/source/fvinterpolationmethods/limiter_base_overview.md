This object provides a second-order, Total Variation Diminishing (TVD)-style scheme for
interpolating a cell-centered
advected quantity to a finite-volume face on unstructured grids. It is formulated as a *limited
blending weight* between upwind and linear (geometric) interpolation, so it can be assembled using
only two-cell (elem/neighbor) matrix weights ([!cite](moukalled2016finite), [!cite](jasak1996error), [!cite](greenshieldsweller2022), [!cite](harten1997)).

Let $\phi_U$ and $\phi_D$ denote the upwind and downwind cell-centered values on a face (as
determined by the sign of the face mass flux). The face value is written in a Normalized Variable
Diagram (NVD) / Greenshields-type blending form
([!cite](greenshieldsweller2022), [!cite](jasak1996error)):

!equation
\phi_f = (1-g)\,\phi_U + g\,\phi_D,

where $g$ controls how much downwind information is admitted. This method computes

!equation
g = \alpha\,\beta(r_f)\,(1-w_f),

with:

- $\alpha$ the user scaling factor ([!param](/FVInterpolationMethods/FVAdvectedMinmodWeightBased/blending_factor)).
  $\alpha=0$ gives pure upwind; $\alpha=1$ gives the full limited blending. Values $<1$ can be
  useful for improving robustness of fully implicit solves.
- $w_f$ the geometric *linear-interpolation* weight associated with the upwind cell. On a uniform
  orthogonal mesh, $w_f=\tfrac{1}{2}$ and the linear scheme is recovered when $g=1-w_f$.
- $\beta(r_f)$ the limiter coefficient computed from a smoothness indicator $r_f$ that compares
  upwind and downwind variation.

When [!param](/FVInterpolationMethods/FVAdvectedMinmodWeightBased/limit_to_linear) is enabled (default),
the blending is additionally constrained so the scheme is never more downwind-biased than linear
interpolation:

!equation
0 \le g \le 1-w_f.

For unstructured grids, $r_f$ is typically formed using a virtual upwind state built from the
upwind cell gradient and the vector connecting neighboring cell centroids (see
[!cite](moukalled2016finite)). In smooth regions ($r_f\approx 1$), $\beta\approx 1$ and the method
approaches linear interpolation. At local extrema or discontinuities ($r_f\le 0$), $\beta=0$ and
the method reverts to upwind, suppressing non-physical oscillations.
