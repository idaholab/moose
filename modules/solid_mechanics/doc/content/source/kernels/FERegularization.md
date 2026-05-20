# FERegularization

!syntax description /Kernels/FERegularization

## Description

`FERegularization` adds a finite-element regularization term to a scalar field. The
kernel is provided by `SolidMechanicsApp`, but the operator is not tied to a specific
constitutive model or contact formulation.

!alert warning title=Second-derivative FE data required
This kernel evaluates second derivatives of the trial and test functions. Use an FE
family and order for which libMesh provides meaningful second derivatives on the selected
elements; the example inputs use `family = LAGRANGE` and `order = SECOND`. Global $C^1$
continuity is not required by this kernel, but variables whose second derivatives vanish
will produce little or no regularization contribution.

The included tests and intended displacement-regularization use case run on $C^0$
quadratic Lagrange elements. This is non-conforming for a standalone biharmonic solve,
but the kernel is used as a regularization contribution rather than as a conforming
biharmonic discretization.

For variable $u$, test function $v$, coefficient $k$, and mesh dimension $d$, the
supported options are

!equation
R_i^{\mathrm{HuHu}} = \int_\Omega k \, u_{,jk} v_{i,jk} \, d\Omega ,

!equation
R_i^{\mathrm{LuLu}} = \int_\Omega k \, \Delta u \Delta v_i \, d\Omega ,

and

!equation
R_i^{\mathrm{HuHu-LuLu}} =
\int_\Omega k \left( u_{,jk} v_{i,jk} - c_L \Delta u \Delta v_i \right) d\Omega .

Here $u_{,jk}$ and $v_{i,jk}$ are Hessian components, $\Delta u$ and
$\Delta v_i$ are Laplacians, and $c_L$ is [!param](/Kernels/FERegularization/lulu_factor).
The names HuHu and LuLu follow terminology used for third-medium contact regularization,
but the kernel is a general finite-element regularization operator.

For [!param](/Kernels/FERegularization/regularization) set to `huhu_lulu`,
[!param](/Kernels/FERegularization/lulu_factor) defaults to $1 / d$. Values larger
than $1 / d$ are accepted with a warning because they may cause negative strain-energy
contributions.

The `huhu_lulu` option is rejected in one dimension because HuHu and LuLu are identical
when the default LuLu factor is used. Use `huhu` or `lulu` directly for one-dimensional
problems.

For vector-valued mechanics fields, add one scalar regularization kernel per displacement
component:

```text
[Kernels]
  [regularize_x]
    type = FERegularization
    variable = disp_x
    regularization = huhu_lulu
    coefficient = 1
  []
  [regularize_y]
    type = FERegularization
    variable = disp_y
    regularization = huhu_lulu
    coefficient = 1
  []
[]
```

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/fe_regularization/fe_regularization_default_factor.i block=Kernels

## References

- Frederiksen, A. H., Dalklint, A., Sigmund, O., and Poulios, K. (2025). "Improved
  third medium formulation for 3D topology optimization with contact." Computer Methods in
  Applied Mechanics and Engineering, 436, Article 117595.
  [doi:10.1016/j.cma.2024.117595](https://doi.org/10.1016/j.cma.2024.117595)
- Bluhm, G. L., Sigmund, O., and Poulios, K. (2021). "Internal contact modeling for
  finite strain topology optimization." Computational Mechanics, 67, 1099-1114.
  [doi:10.1007/s00466-021-01974-x](https://doi.org/10.1007/s00466-021-01974-x)
- [Third medium contact method](https://en.wikipedia.org/wiki/Third_medium_contact_method)

!syntax parameters /Kernels/FERegularization

!syntax inputs /Kernels/FERegularization

!syntax children /Kernels/FERegularization
