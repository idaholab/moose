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

!alert note
This functionality currently only works for the `Constraints` system. Adding it to the `ContactAction` remains an ongoing process.

To successfully use this approach, one needs to enable dual shape function interpolation of the Lagrange multiplier variables (`use_dual = true`). Additionally, one needs to define a dummy auxiliary variable on the secondary interface that utilizes standard test/shape functions. An example syntax is as follows:


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
