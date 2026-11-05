# Petrov-Galerkin approach for Lagrange multipliers

## Overview

The Petrov–Galerkin method is acknowledged to be a numerical method used to approximate solutions of partial differential equations where the test function and solution function belong to different function spaces.

For mortar-based mechanical contact, when dual shape function is utilized for the Lagrange multiplier variable, we may end up with a negative weighted gap value at certain nodes. This negative weighted gap value not only contradicts the continuum setting, but also may lead to unacceptable errors or even a non-converging active set strategy.

In order to resolve this issue, a Petrov–Galerkin interpolation for the Lagrange multiplier is proposed (see e.g., [!cite](popp2014dual)). Specifically, dual shape functions are employed for the Lagrange multiplier field, thus resulting in the desired diagonal structure of mortar matrix, which allows for the condensation of the discrete Lagrange multiplier degrees of freedom (see [](VariableCondensationPreconditioner.md)). On the other hand, the interpolation of its variation in the constraint equations is done by standard shape functions. To this end, this approach combines the strengths of both standard and dual Lagrange multiplier interpolation methods.
Mathematically, this means

\begin{equation}
\begin{split}
\lambda & = \sum_{i=0}^{n_\lambda} \Phi_i \lambda_i,\\
\delta \lambda & = \sum_{i=0}^{n_\lambda} N_i \delta\lambda_i,
\end{split}
\end{equation}
where $\lambda$ and $\delta \lambda$ are the Lagrange multiplier variable and its variation, respectively.  The $n_\lambda$ is the total number of nodes along the secondary contact interface. The $\lambda_i$ and $\delta\lambda_i$ are the discretized nodal values of the Lagrange multiplier variable and its variation, respectively. The $\Phi_i$ is the dual basis function, the $N_i$ is the standard basis function.

## Implementation and Usage Details

The Petrov-Galerkin method is implemented in MOOSE by overwriting the Lagrange multiplier's (dual) test functions using that of a standard Lagrange multiplier variable. This functionality is enabled for all mortar-based constraints and the weighted-gap user object.

To successfully use this approach in the contact action, one just needs to set `use_petrov_galerkin = true` in the `Contact` block. For example:

```
[Contact]
  [mortar]
    primary = 'bottom_top'
    secondary = 'top_bottom'
    formulation = mortar
    model = coulomb
    friction_coefficient = 0.4
    c_normal = 1e4
    c_tangential = 1.0e4
    use_petrov_galerkin = true
  []
[]
```

To successfully use this approach in `Constraints` based input, one needs to enable dual shape function interpolation of the Lagrange multiplier variables (`use_dual = true`). Additionally, one needs to define a dummy auxiliary variable on the secondary interface that utilizes standard test/shape functions. An example syntax is as follows:


```
[AuxVariables]
  [aux_lm]
    block = 'secondary_lower'
    order = SECOND # needs to be consistent with the Lagrange multiplier variable
    use_dual = false
  []
[]
```

Then enable Petrov-Galerkin in mortar-based Constraint as follows:

```
[Constraints]
  [example_mortar_constraint]
    type = ...
    ...
    ...
    use_petrov_galerkin = true
    aux_lm = aux_lm
  []
[]
```

Similarly, in the `LMWeightedGapUserObject`:

```
[UserObjects]
  [weighted_gap_uo]
    type = LMWeightedGapUserObject
    ...
    ...
    use_petrov_galerkin = true
    aux_lm = aux_lm
  []
[]
```


!alert note
One can enable Petrov-Galerkin for mortar-based constraints not limited to mechanical contact (e.g., gap conductance). It is up to the user's discretion about the meaningful usage of Petrov-Galerkin interpolation for the Lagrange multipliers.

## Piecewise-linear basis for second-order faces

On QUAD8 and TRI6 faces, the faces of HEX20 and TET10 elements, the standard basis $N_i$ is not
non-negative and its nodal integrals are not positive ($-1/3$ at QUAD8 corners, $0$ at TRI6
vertices). The weighted gap can then have the wrong sign, and MOOSE reports an error if
Petrov-Galerkin is combined with a SECOND order dual Lagrange multiplier on these faces (see
[Transformed_dual_basis_for_higher_order_mortar.md]).

Instead, the gap can be weighted with the piecewise-linear basis of [!cite](carvalho2023efficient),
Eq. (43). Each second-order face is divided into the linear sub-elements used for mortar
segmentation, and $\delta \lambda$ is interpolated with the first-order Lagrange functions of those
sub-elements,

\begin{equation}
\delta \lambda = \sum_{i=0}^{n_\lambda} N^{\mathrm{sub}}_i \delta\lambda_i .
\end{equation}

Every face node keeps its own Lagrange multiplier, and the functions $N^{\mathrm{sub}}_i$ are
non-negative and form a partition of unity, so a positive gap always gives a positive weighted gap.
The Lagrange multiplier keeps its dual basis. The reference-element integrals of $N^{\mathrm{sub}}_i$
are:

| Face  | Corner | Mid-edge | Center |
| ----- | ------ | -------- | ------ |
| EDGE3 | $1/2$  | $1$      |        |
| TRI6  | $1/24$ | $1/8$    |        |
| QUAD8 | $1/6$  | $5/6$    |        |
| QUAD9 | $1/4$  | $1/2$    | $1$    |

In three dimensions, mortar segments already lie within single sub-elements. In two dimensions,
mortar segments of EDGE3 faces are split at the mid node, so that the weighted gap is integrated
exactly on every segment.

In the contact action, set [!param](/Contact/ContactAction/petrov_galerkin_test) to
`PIECEWISE_LINEAR` together with [!param](/Contact/ContactAction/use_petrov_galerkin). This also
applies to `mortar_dynamics = true`:

!listing modules/contact/test/tests/petrov_galerkin_piecewise_linear/tilted_gap_3d.i block=Contact

In `Constraints` based input, set `use_piecewise_linear_basis = true` on the SECOND order
auxiliary variable that is passed as `aux_lm`. In two dimensions, every mortar object on the
interface must also set `split_segments_at_sub_elements = true`.

The piecewise-linear basis is available on EDGE3, TRI6, QUAD8 and QUAD9 secondary faces with
Lagrange multiplier mortar contact. It applies only to the lower-dimensional shape functions of the
variable.

## References

!bibtex bibliography
