# ADFERegularization

!syntax description /Kernels/ADFERegularization

## Description

`ADFERegularization` is the automatic-differentiation version of
[FERegularization.md]. It supports the same HuHu, LuLu, and HuHu-LuLu finite-element
regularization options and computes Jacobian contributions through MOOSE's AD
infrastructure.

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

For `regularization = huhu_lulu`, [!param](/Kernels/ADFERegularization/lulu_factor)
defaults to $1 / d$, where $d$ is the mesh dimension. Values larger than $1 / d$
are accepted with a warning because they may cause negative strain-energy contributions.

The `huhu_lulu` option is rejected in one dimension because HuHu and LuLu are identical
when the default LuLu factor is used. Use `huhu` or `lulu` directly for one-dimensional
problems.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/fe_regularization/ad_fe_regularization_default_factor.i block=Kernels

!syntax parameters /Kernels/ADFERegularization

!syntax inputs /Kernels/ADFERegularization

!syntax children /Kernels/ADFERegularization
