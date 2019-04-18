# INSADMomentumAdvection

This object adds the $\rho\left(\vec u \cdot\nabla\right)\vec u$ term of the
incompressible Navier Stokes momentum equation. Note that in the code we right
multiply by $\vec u$, e.g. we perform $\nabla\vec u \cdot \vec u$ because
$\nabla \vec u$ is stored in [Jacobian matrix form](https://en.wikipedia.org/wiki/Jacobian_matrix_and_determinant).

!syntax description /Kernels/INSADMomentumAdvection

!syntax parameters /Kernels/INSADMomentumAdvection

!syntax inputs /Kernels/INSADMomentumAdvection

!syntax children /Kernels/INSADMomentumAdvection
