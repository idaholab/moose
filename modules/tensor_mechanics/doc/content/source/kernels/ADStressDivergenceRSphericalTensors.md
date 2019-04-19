# ADStressDivergenceRSphericalTensors

!syntax description /Kernels/ADStressDivergenceRSphericalTensors

## Description

The kernel `ADStressDivergenceRSphericalTensors` solves the stress divergence
equation for a spherically symmetric system on a 1D mesh. Forward mode automatic
differentiation is used to compute an exact Jacobian.

## Residual Calculation

!include modules/tensor_mechanics/common/supplementalADStressDivergenceKernels.md

The stress divergence in spherical coordinates includes contributions from the
normal polar and azimuthal stresses even in the 1D case.  After simplifying for
the 1D case, the spherical stress divergence reduces to

\begin{equation}
\label{eqn:strongformspherical}
\nabla \sigma  =  \left[ \frac{\partial \sigma_{rr}}{\partial r} + \frac{2}{X_r} \sigma_{rr} - \frac{1}{X_r} \left( \sigma_{\phi \phi} + \sigma_{\theta \theta} \right)  \right] \hat{e}_r
\end{equation}

In deriving the weak form of this equation, the second term in
[eqn:strongformspherical] goes to zero and the residual contribution in the
`ADStressDivergenceRSphericalTensors` kernel becomes

\begin{equation}
\boldsymbol{R} = \sigma_{rr} \frac{ \partial \phi_i }{ \partial r} + \frac{ \phi_i}{X_r} \left( \sigma_{\phi \phi} + \sigma_{\theta \theta} \right)
\end{equation}

!alert note title=Use `RSPHERICAL` Coordinate Type
The coordinate type in the `[Problem]` block of the input file must be set to
`coord_type = RSPHERICAL`.

!syntax parameters /Kernels/ADStressDivergenceRSphericalTensors

!include modules/tensor_mechanics/common/seealsoADStressDivergenceKernels.md

!syntax inputs /Kernels/ADStressDivergenceRSphericalTensors

!syntax children /Kernels/ADStressDivergenceRSphericalTensors
