#StressDivergenceRSphericalTensors
!description /Kernels/StressDivergenceRSphericalTensors


##Description
This kernel solves the steady state stress divergence equation in polar coordinates on a 1D mesh.

!!! info
    The `COORD_TYPE` in the Problem block of the input file must be set to RSPHERICAL.

!include docs/content/documentation/modules/tensor_mechanics/common/supplementalStressDivergenceKernels.md

For a detailed explanation of the settings for `use_displaced_mesh` in mechanics problems, see the [StressDivergence](/tensor_mechanics/StressDivergence.md) page.

 The stress divergence in spherical coordinates includes contributions from the normal polar and azimuthal stresses even in the 1D case.  After simplifying for the 1D case, the spherical stress divergence reduces to
$$
\nabla \sigma  =  \left[ \frac{\partial \sigma_{rr}}{\partial r} + \frac{2}{X_r} \sigma_{rr} - \frac{1}{X_r} \left( \sigma_{\phi \phi} + \sigma_{\theta \theta} \right)  \right] \hat{e}_r
$$

In deriving the weak form of this equation, the second term drops out so that the residual contribution in the `StressDivergenceRSphericalTensors` kernel is
$$
\mathbf{R} = \sigma_{rr} \frac{ \partial \phi_i }{ \partial r} + \frac{ \phi_i}{X_r} \left( \sigma_{\phi \phi} + \sigma_{\theta \theta} \right)
$$

!parameters /Kernels/StressDivergenceRSphericalTensors

!inputfiles /Kernels/StressDivergenceRSphericalTensors

!childobjects /Kernels/StressDivergenceRSphericalTensors
