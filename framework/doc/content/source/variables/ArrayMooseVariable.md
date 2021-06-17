# ArrayMooseVariable

!syntax description /Variables/ArrayMooseVariable

## Overview

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

| OutputType          | OutputShape           | OutputData |
| :- | :- | :- |
| Real                | Real                  | Real |
| RealVectorValue     | RealVectorValue       | Real |
| RealEigenVector     | Real                  | RealEigenVector |

The three rows correspond to standard, vector and array variables.
OutputType is the data type used for templating.
RealEigenVector is a typedef in [MooseTypes.h] as *Eigen::Matrix<Real, Eigen::Dynamic, 1>*.
OutputShape is for the type of shape functions and OutputData is the type of basis function expansion coefficients that are stored in the moose array variable grabbed from the solution vector.

## Kernels for Array Variables

Since array variables have a different OutputData type, standard Kernels cannot be used on array variables. Kernels for array variables, or ArrayKernels, must derive from `ArrayKernel` which have different virtual functions for `computeQpResidual`, `computeQpJacobian`, and `computeQpOffDiagJacobian`. The declaration of these functions are below:

!listing language=cpp
virtual void computeQpResidual(RealEigenVector & residual) = 0;
virtual RealEigenVector computeQpJacobian();
virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFEBase & jvar);

When defining a `computeQpResidual` in a derived class, this function +must+ define the residual in the input arguement (`residual`). This input is already properly sized when called in ArrayKernel.C. `computeQpJacobian` must return a vector defining the on-diagonal terms of the Jacobian. `computeQpOffDiagJacobian` must return a matrix with number of rows equal to the number of components and number of columns being the number of components in `jvar.`

Using [ArrayDiffusion.md] as an example. The `computeQpResidual` function has

!listing ArrayDiffusion.C re=void\sArrayDiffusion::computeQpResidual.*?^}

where `_grad_u[_qp]` is an `Eigen::Matrix` with number of rows being equal to the number of components of the array variable and number of columns being `LIBMESH_DIM`. `_array_grad_test[_i][_qp]` is an `Eigen::Map` of classic `_grad_test[_i][_qp]` which is in type of `Gradient`. Thanks to the Eigen matrix arithmetic operators, we can have a simple multiplication expression here. `_d` is a pointer of a material property of `Real` type for scalar diffusion coefficient. Here we assume the diffusion coefficient is the same for all components. `_d_array` is a pointer to a `RealEigenVector` material
property, here we assume there is a diffusion coefficient for each component with no
coupling. `_d_2d_array` is a pointer to a `RealEigenMatrix` material property, where
the diffusion coefficient is represented as dense matrix. See [ArrayDiffusion.md] for more details.

Correspondingly the `computeQpJacobian` has

!listing ArrayDiffusion.C re=RealEigenVector\sArrayDiffusion::computeQpJacobian.*?^}

It is noted that only the diagonal entries of the diffusion coefficients are used in the fully-coupled case because `computeQpJacobian` is supposed to only assemble the block-diagonal part of the Jacobian.
The full local Jacobian is assembled in function `computeQpOffDiagJacobian`, where when the off-diagonal variable is the array variable, we have

!listing ArrayDiffusion.C
  re=RealEigenMatrix\sArrayDiffusion::computeQpOffDiagJacobian.*?^}

The retuned value is in type of an Eigen matrix with number of rows and columns equal to the number of components.

`ArrayKernel` also has virtual functions including:

!listing language=cpp
virtual void initQpResidual();
virtual void initQpJacobian();
virtual void initQpOffDiagJacobian(const MooseVariableFEBase & jvar);

Which are functions that are called inside the quadrature point loop, but outside the test/shape function loop. This is useful to perform operations that depend on position (quadrature point) but do not depend on the test/shape function. For instance, [ArrayDiffusion.md] uses `initQpResidual` to check if the size of the vector or matrix diffusion coefficient matches the number of components in the variable:

!listing ArrayDiffusion.C re=void\sArrayDiffusion::initQpResidual.*?^}

Future work:

- To change the current dof ordering for elemental variables so that we can avoid bunch of if statements with `isNodal()` in `MooseVariableFE.C`, (refer to [libMesh Issue 2114](https://github.com/libMesh/libmesh/issues/2114)).
- To use Eigen::Map for faster solution vector access.
- To implement ArrayInterfaceKernel and ArrayConstraints.

## Useful Eigen API

Linear algebra is very easy using Eigen, it has a lot of API for matrix arithmetic and manipulation that simplifies code and helps developers avoid writing their own loops. For a full list of functions, visit the [Eigen matrix doxygen](http://eigen.tuxfamily.org/dox/classEigen_1_1Matrix.html), which is relevant to the `RealEigenVector` and `RealEigenMatrix` types in MOOSE. Below is a list of commonly used functions for residual and jacobian evaluations. For exposition here is a definition of a vector and matrix:

!equation
\vec{v} =
\begin{bmatrix}
v_1 \\ v_2 \\ \vdots \\ v_n
\end{bmatrix}
\quad , \quad
M =
\begin{bmatrix}
m_{11} & m_{12} & \dots & m_{1n} \\
m_{21} & m_{22} & \dots & m_{2n} \\
\vdots & \vdots & \ddots & \vdots \\
m_{n1} & m_{n2} & \dots & m_{nn}
\end{bmatrix}
.

- Set all elements to same value:

  !equation
  \vec{v}\texttt{.setZero()} \rightarrow v_i = 0, i=1,...,n

  !equation
  \vec{v}\texttt{.setOnes()} \rightarrow v_i = 1, i=1,...,n

  !equation
  \vec{v}\texttt{.setConstant(}a\texttt{)} \rightarrow v_i = a, i=1,...,n

- Change matrix representation:

  !equation
  \vec{v}\texttt{.asDiagonal()} \rightarrow
  \begin{bmatrix}
  v_1 &     &        &     \\
      & v_2 &        &     \\
      &     & \ddots &     \\
      &     &        & v_n \\
  \end{bmatrix}

  !equation
  M\texttt{.diagonal()} \rightarrow
  \begin{bmatrix}
  m_{11} \\ m_{22} \\ \vdots \\ m_{nn}
  \end{bmatrix}

- Element-wise operations:

  !equation
  \vec{v} \texttt{ = } \vec{v}\texttt{.cwiseProduct(}\vec{w}\texttt{)} \rightarrow v_i = v_iw_i, i=1,...,n

  !equation
  \vec{v}\texttt{.array() /= } \vec{w}\texttt{.array()} \rightarrow v_i = v_i/w_i, i=1,...,n

## Eigen Tips and Tricks (Advanced)

Eigen has some unique features that, when used properly, can significantly impact performance. Here are some recommendations that can improve code performance.

- [Aliasing](http://eigen.tuxfamily.org/dox/group__TopicAliasing.html) is a technique in Eigen that contructs temporary objects when performing matrix multiplications, this is to avoid overriding data that needs to be used later in the computation. For instance `vec = mat * vec` will create a temporary vector for `mat * vec` then assign it to `vec` at the end. However, `vec2 = mat * vec1` does not need this temporary object and assign the result to `vec2` directly, this aliasing can be avoided by doing `vec2.noalias()`. The `noalias()` function should be used with extreme caution since it can cause erroneous results.

- Eigen uses what's known as [expression templates](https://en.wikipedia.org/wiki/Expression_templates), enabling operations to be known at compile time. This allows multiple operations to occur in a single element element loop, providing more compiler optimization and improved cache efficiency. With this in mind, it is often better to write multiple Eigen operations in a single line or assignment. For instance, with the following syntax:

  !listing! language=cpp
  a = 3*b + 4*c + 5*d
  !listing-end!

  Eigen will compile to a single for loop:

  !listing! language=cpp
  for (unsigned int i = 0; i < b.size(); ++i)
    a[i] = 3*b[i] + 4*c[i] + 5*d[i];
  !listing-end!

- Eigen also has a interface for accessing raw buffers using its [Map class](http://eigen.tuxfamily.org/dox/group__TutorialMapClass.html).

For a better understanding of Eigen and using it to its full potential, it is highly recommended to go through Eigen's [tutorials](http://eigen.tuxfamily.org/dox/modules.html).


!syntax parameters  /Variables/ArrayMooseVariable

!syntax inputs /Variables/ArrayMooseVariable

!syntax children /Variables/ArrayMooseVariable
