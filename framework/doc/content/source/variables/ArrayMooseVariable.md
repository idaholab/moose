# ArrayMooseVariable

An array variable is define as a set of standard field variables with the same finite element family and order.
Each standard variable of an array variable is referred to as a component of the array variable.
An array kernel is a MOOSE kernel operating on an array variable and assembles the residuals and Jacobians for all the components of the array variable.
A sample array kernel can be found at [ArrayDiffusion.md].
The purpose of having array kernels is to reduce the number of kernels when the number of components of an array variable is large (potentially hundreds or thousands) and to avoid duplicated operations otherwise with lots of standard kernels.
Array kernel can be useful for radiation transport where an extra independent direction variable can result into large number of standard variables.
Similarly as array kernel, we have array initial conditions, array boundary conditions, array DG kernels, array interface kernels, array constraints, etc.

The design:

- to use variable groups in [libMesh] to group components in an array variable together.
- to use template to avoid code duplication with standard and vector variables.
- to use [Eigen](https://eigen.tuxfamily.org/dox/group__QuickRefPage.html)::Matrix to hold local dofs, solutions on quadrature points for an array variable to ease the local operations.
- to use dense matrices or vectors as standard or vector variables with proper sizes for holding the local Jacobians and residuals that are to be assembled into a global Jacobian matrix and a global residual vector.

The following map is useful for understanding the template:

| OutputType          |OutputShape           |OutputData|
|-|-|-|
| Real                |Real                  |Real|
| RealVectorValue     |RealVectorValue       |Real|
| RealEigenVector     |Real                  |RealEigenVector|

The three rows correspond to standard, vector and array variables.
OutputType is the data type used for templating.
RealEigenVector is a typedef in [MooseTypes.h] as *Eigen::Matrix<Real, Eigen::Dynamic, 1>*.
OutputShape is for the type of shape functions and OutputData is the type of basis function expansion coefficients that are stored in the moose array variable grabbed from the solution vector.

The final form of an array kernel is quite simple. Using `ArrayDiffusion` as an example. The `computeQpResidual` function has

```
  return _grad_u[_qp] * _array_grad_test[_i][_qp] * (*_d)[_qp];
```

where `_grad_u[_qp]` is an `Eigen::Matrix` with number of rows being equal to the number of components of the array variable and number of columns being `LIBMESH_DIM`. `_array_grad_test[_i][_qp]` is an `Eigen::Map` of classic `_grad_test[_i][_qp]` which is in type of `Gradient`. Thanks to the Eigen matrix arithmetic operators, we can have a simple multiplication expression here. `_d` is a pointer of a material property of `Real` type for scalar diffusion coefficient. Here we assume the diffusion coefficient is the same for all components. The returned value is RealEigenVector, i.e. an Eigen vector.

Of course, if we have different diffusion coefficients for different components, we will have something like

```
    RealEigenVector v = _grad_u[_qp] * _array_grad_test[_i][_qp];
    for (unsigned int i = 0; i < _var.count(); ++i)
      v(i) *= (*_d_array)[_qp](i);
```

`_var.count()` gives the number of components of the array variable.
If we have coupled diffusion terms, i.e. diffusion coefficient is a matrix, we will have

```
  return (*_d_2d_array)[_qp] * (_grad_u[_qp] * _array_grad_test[_i][_qp]);
```

Correspondingly the `computeQpJacobian` has

```
    return RealEigenVector::Constant(_var.count(),
                                     _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d)[_qp]);
```

for a scalar diffusion coefficient or

```
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_array)[_qp];
```

for an array diffusion coefficient or

```
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_2d_array)[_qp].diagonal();
```

for a matrix diffusion coefficient.

It is noted that only the diagonal entries of the diffusion coefficients are used in the fully-coupled case because `computeQpJacobian` is supposed to only assemble the block-diagonal part of the Jacobian.
The full local Jacobian is assembled in function `computeQpOffDiagJacobian`, where when the off-diagonal variable is the array variable, we have

```
  return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_2d_array)[_qp];
```

The retuned value is in type of an Eigen matrix with number of rows and columns equal to the number of components.

Future work:

- To profile the code with large number of variables to find the hot spots and fix them if there are any.
- To change the current dof ordering for elemental variables so that we can avoid bunch of if statements with `isNodal()` in `MooseVariableFE.C`, (refer to [libMesh Issue 2114](https://github.com/libMesh/libmesh/issues/2114).
- To use Eigen::Map for faster solution vector access.
- To implement ArrayInterfaceKernel and ArrayConstraints.
