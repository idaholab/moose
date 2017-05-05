#Stress Divergence RZ Tensors
!description /Kernels/StressDivergenceRZTensors



##Description
This kernel solves the steady state stress divergence equation in cylindrical coordinates for a 2D mesh.

!!! info
    The `COORD_TYPE` in the Problem block of the input file must be set to RZ.

!include docs/content/documentation/modules/tensor_mechanics/common/supplementalStressDivergenceKernels.md

For a detailed explanation of the settings for `use_displaced_mesh` in mechanics problems, see the [Introduction/StressDivergence](auto::/introduction/StressDivergence) page.

As in the Stress Divergence kernel for Cartesian coordinates, [StressDivergenceTensors](/StressDivergenceTensors.md), the stress divergence kernel for the Axisymmetric simulations includes the stress divergence calculation for the residual and the calculation of the Jacobian matrix.

In cylindrical coordinates, the [divergence of a rank-2 tensor](https://en.wikipedia.org/wiki/Tensor_derivative_%28continuum_mechanics%29#Cylindrical_polar_coordinates_2) includes mixed term contributions.  In the axisymmetric model we assume symmetric loading conditions, in addition to the zero out-of-plane shear strains, so that the residual computation is simplified.

$$
\begin{eqnarray}
\nabla \sigma  & = & \left[ \frac{\partial \sigma_{rr}}{\partial r} + \frac{u_r}{X_r}\sigma_{\theta \theta} + \frac{\partial \sigma_{rz}}{\partial z} \right] \hat{e}_r \\
 & + & \left[ \frac{\partial \sigma_{zz}}{\partial z} + \frac{\partial \sigma_{rz}}{\partial r}    \right] \hat{e}_z
\end{eqnarray}
$$

!!! note
    This calculation of the residual and the Jacobian calculation require the axis of symmetry lies along the $z$-axis.

The calculation of the Jacobian is similarly complex, requiring up to four terms in the calculation of the diagonal entries.


!parameters /Kernels/StressDivergenceRZTensors

!inputfiles /Kernels/StressDivergenceRZTensors

!childobjects /Kernels/StressDivergenceRZTensors
