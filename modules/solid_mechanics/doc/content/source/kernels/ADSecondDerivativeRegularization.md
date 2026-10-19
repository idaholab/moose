# ADSecondDerivativeRegularization

!syntax description /Kernels/ADSecondDerivativeRegularization

## Description

`ADSecondDerivativeRegularization` is the automatic-differentiation version of
[SecondDerivativeRegularization.md]. It supports the same HuHu, LuLu, and HuHu-LuLu
second-derivative regularization options and computes Jacobian contributions through MOOSE's AD
infrastructure.

!include modules/solid_mechanics/common/supplementalSecondDerivativeRegularization.md

For `regularization = huhu_lulu`, [!param](/Kernels/ADSecondDerivativeRegularization/lulu_factor)
defaults to $1 / d$, where $d$ is the mesh dimension. Values larger than $1 / d$
are accepted with a warning because they may cause negative strain-energy contributions.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/second_derivative_regularization/scalar_regularization.i block=Kernels replace=['${reg_kernel}','ADSecondDerivativeRegularization']

!syntax parameters /Kernels/ADSecondDerivativeRegularization

!syntax inputs /Kernels/ADSecondDerivativeRegularization

!syntax children /Kernels/ADSecondDerivativeRegularization
