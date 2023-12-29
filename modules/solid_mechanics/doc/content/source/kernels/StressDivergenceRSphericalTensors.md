# Stress Divergence RSpherical Tensors

!syntax description /Kernels/StressDivergenceRSphericalTensors

## Description

The kernel `StressDivergenceRSphericalTensors` solves the stress divergence equation for a
spherically symmetric system on a 1D mesh.

!alert note
The `COORD_TYPE` in the Problem block of the input file must be set to RSPHERICAL.

The `StressDivergenceRSphericalTensors` kernel can be automatically created with the
[TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md). Use of the tensor
mechanics master action is recommended to ensure the consistent setting of the *use_displaced_mesh*
parameter for the strain formulation selected.  For a detailed explanation of the settings for
_use_displaced_mesh_ in mechanics problems and the TensorMechanics Master Action usage, see the
[Introduction/StressDivergence](/StressDivergence.md) page.

## Residual Calculation

!include modules/tensor_mechanics/common/supplementalStressDivergenceKernels.md

The stress divergence in spherical coordinates includes contributions from the normal polar and
azimuthal stresses even in the 1D case.  After simplifying for the 1D case, the spherical stress
divergence reduces to
\begin{equation}
\label{eqn:strongformspherical}
\nabla \sigma  =  \left[ \frac{\partial \sigma_{rr}}{\partial r} + \frac{2}{X_r} \sigma_{rr} - \frac{1}{X_r} \left( \sigma_{\phi \phi} + \sigma_{\theta \theta} \right)  \right] \hat{e}_r
\end{equation}

In deriving the weak form of this equation, the second term in [eqn:strongformspherical]
goes to zero and the residual contribution in the `StressDivergenceRSphericalTensors` kernel becomes
\begin{equation}
\boldsymbol{R} = \sigma_{rr} \frac{ \partial \phi_i }{ \partial r} + \frac{ \phi_i}{X_r} \left( \sigma_{\phi \phi} + \sigma_{\theta \theta} \right)
\end{equation}

## Example Input File syntax

Using the tensor mechanics master action, as shown

!listing modules/tensor_mechanics/test/tests/1D_spherical/finiteStrain_1DSphere_hollow.i
         block=Modules/TensorMechanics/Master

the `StressDivergenceRSphericalTensors` kernel will be automatically built when the coordinate system
in the Problem block is specified for the spherical system,

!listing modules/tensor_mechanics/test/tests/1D_spherical/finiteStrain_1DSphere_hollow.i block=Problem

and only a single displacement variable is provided:

!listing modules/tensor_mechanics/test/tests/1D_spherical/finiteStrain_1DSphere_hollow.i block=GlobalParams

!syntax parameters /Kernels/StressDivergenceRSphericalTensors

!include modules/tensor_mechanics/common/seealsoADStressDivergenceKernels.md

!syntax inputs /Kernels/StressDivergenceRSphericalTensors

!syntax children /Kernels/StressDivergenceRSphericalTensors
