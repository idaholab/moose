# DisplacementRegularization

!syntax description /Kernels/DisplacementRegularization

## Description

`DisplacementRegularization` adds a regularization term to one displacement component in
the solid mechanics displacement equation. For vector-valued displacement variables, add
one scalar kernel per component.

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

For displacement component $u$, test function $v$, coefficient $k$, and mesh dimension
$d$, the supported options are

!equation
R_i^{\mathrm{HuHu}} = \int_\Omega k \, u_{,jk} v_{i,jk} \, d\Omega ,

!equation
R_i^{\mathrm{LuLu}} = \int_\Omega k \, \Delta u \Delta v_i \, d\Omega ,

and

!equation
R_i^{\mathrm{HuHu-LuLu}} =
\int_\Omega k \left( u_{,jk} v_{i,jk} - c_L \Delta u \Delta v_i \right) d\Omega .

Here $u_{,jk}$ and $v_{i,jk}$ are Hessian components, $\Delta u$ and
$\Delta v_i$ are Laplacians, and $c_L$ is [!param](/Kernels/DisplacementRegularization/lulu_factor).
The names HuHu and LuLu follow terminology used for third-medium contact regularization
[!cite](frederiksen2025improved,bluhm2021internal), but the kernel only contributes the
regularization term and is not tied to contact-specific infrastructure.

For [!param](/Kernels/DisplacementRegularization/regularization) set to `huhu_lulu`,
[!param](/Kernels/DisplacementRegularization/lulu_factor) defaults to $1 / d$. Values larger
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
    type = DisplacementRegularization
    variable = disp_x
    regularization = huhu_lulu
    coefficient = 1
  []
  [regularize_y]
    type = DisplacementRegularization
    variable = disp_y
    regularization = huhu_lulu
    coefficient = 1
  []
[]
```

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/displacement_regularization/displacement_regularization_default_factor.i block=Kernels

!syntax parameters /Kernels/DisplacementRegularization

!syntax inputs /Kernels/DisplacementRegularization

!syntax children /Kernels/DisplacementRegularization

!bibtex bibliography
