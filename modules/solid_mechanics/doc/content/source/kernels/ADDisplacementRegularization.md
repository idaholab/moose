# ADDisplacementRegularization

!syntax description /Kernels/ADDisplacementRegularization

## Description

`ADDisplacementRegularization` is the automatic-differentiation version of
[DisplacementRegularization.md]. It supports the same HuHu, LuLu, and HuHu-LuLu
displacement regularization options and computes Jacobian contributions through MOOSE's AD
infrastructure.

!include modules/solid_mechanics/common/supplementalDisplacementRegularization.md

For `regularization = huhu_lulu`, [!param](/Kernels/ADDisplacementRegularization/lulu_factor)
defaults to $1 / d$, where $d$ is the mesh dimension. Values larger than $1 / d$
are accepted with a warning because they may cause negative strain-energy contributions.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/displacement_regularization/ad_displacement_regularization_default_factor.i block=Kernels

!syntax parameters /Kernels/ADDisplacementRegularization

!syntax inputs /Kernels/ADDisplacementRegularization

!syntax children /Kernels/ADDisplacementRegularization
