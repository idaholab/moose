# RayKernel

## Description

`RayKernel` is the base class for the objects that calculate residual vector contributions of non-linear scalar field variables along a [Ray.md]. See also [ADRayKernel.md] for the AD version of this object. The most simple form of a `RayKernel` is a [LineSourceRayKernel.md], which implements a line source along a [Ray.md]. 

The use of a `RayKernel` is the same of a standard [Kernel](Kernels/index.md). In a `RayKernel` subclass the `computeQpResidual()` function must be overridden. This is where you implement your PDE weak form terms. For non-AD objects the following member functions can optionally be overriden:

- `computeQpJacobian()`
- `computeQpOffDiagJacobian()`

These two methods provide extra information that can help the numerical solver(s) converge faster and better.

Inside your `RayKernel` class, you have access to several member variables for computing the residual and Jacobian values in the above mentioned functions:

- `_i`, `_j`: indices for the current test and trial shape functions respectively.
- `_qp`: current quadrature point index.
- `_u`, `_grad_u`: value and gradient of the variable this Kernel operates on;
  indexed by `_qp` (i.e. `_u[_qp]`).
- `_test`, `_grad_test`: value ($\psi$) and gradient ($\nabla \psi$) of the
  test functions at the q-points; indexed by `_i` and then `_qp` (i.e., `_test[_i][_qp]`).
- `_phi`, `_grad_phi`: value ($\phi$) and gradient ($\nabla \phi$) of the
    trial functions at the q-points; indexed by `_j` and then `_qp` (i.e., `_phi[_j][_qp]`).
- `_q_point`: XYZ coordinates of the current quadrature point.
- `_current_elem`: pointer to the current element being operated on.

Also available for override are:

- `precalculateResidual()`
- `precalculateJacobian()`
- `precalculateOffDiagJacobian()`

which are insertion points immedtiately before computing residuals and Jacobians. These methods can be useful when computing cached values that are valid for all quadrature points.

Many other useful member variables exist that describe the [Ray.md] segment. For more information, see [Using a RayKernel](syntax/RayKernels/index.md#using-a-raykernel).

!alert note
RZ and RSPHERICAL coordinate changes are not valid for a `RayKernel`. This is because said coordinate systems have no way to represent a line source - one would end up with a plane/surface source or a volumetric source, respectively. This is why `_coord[_qp]` is not utilized in `RayKernel`.
