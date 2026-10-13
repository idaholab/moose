# Kokkos Kernels System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [Kernels System](syntax/Kernels/index.md) to understand the MOOSE kernel system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE.

The Kokkos-MOOSE kernels are designed to resemble the original MOOSE kernels as much as possible for easier porting and adaptation.
However, some differences still exist due to the fundamentally different programming paradigm between CPU and GPU.
You can create your own kernel by subclassing `Moose::Kokkos::Kernel` as is done in the original MOOSE by inheriting `Kernel`.
However, your kernel should now be registered with `registerKokkosResidualObject()` instead of `registerMooseObject()`.
Also, the signatures of hook methods are different.
In the original MOOSE, the following virtual functions should or optionally have been overriden:

```cpp
virtual Real computeQpResidual() override;
virtual Real computeQpJacobian() override;
virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
virtual Real computeQpOffDiagJacobianScalar(unsigned int jvar) override;
```

Indices such as `_i`, `_j`, and `_qp` were made available in those functions as member variables.

A Kokkos-MOOSE kernel instead computes, once per quadrature point, the coefficients that multiply each test function quantity, and the base class contracts them with the test functions of every degree of freedom.
The residual of a kernel takes the form

!equation
R_i = \sum_{qp} JxW \left( f_v \psi_i + \mathbf{f}_g \cdot \nabla \psi_i \right),

where $f_v$ and $\mathbf{f}_g$ are the coefficients of the test function value and gradient.
Evaluating the coefficients once per quadrature point, independently of the test function index, is what makes this form efficient on GPU, and the [optimized kernels](syntax/Kernels/index.md#optimized) of the original MOOSE use the same idea.

The hook methods are +*no longer virtual functions*+.
Instead, they are defined in the derived class as +*inlined public*+ methods, and their names state the test function quantity they multiply:

```cpp
template <typename Derived>
KOKKOS_FUNCTION Real precomputeQpResidualValue(const unsigned int qp,
                                               AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real3 precomputeQpResidualGrad(const unsigned int qp,
                                               AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real precomputeQpJacobianValue(const unsigned int j,
                                               const unsigned int qp,
                                               AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real precomputeQpOffDiagJacobianValue(const unsigned int j,
                                                      const unsigned int jvar,
                                                      const unsigned int qp,
                                                      AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real precomputeQpOffDiagJacobianScalarValue(const unsigned int j,
                                                            const unsigned int jvar,
                                                            const unsigned int qp,
                                                            AssemblyDatum & datum) const;
```

The Jacobian hooks each have `Value` and `Grad` variants like the residual hooks, with the `Grad` variants returning `Real3`.
A kernel defines the hooks of the terms its weak form contains, and at least one residual hook must be defined.
The base class assembles only the terms whose hooks are defined, so a kernel pays nothing for the terms it does not have, and it assembles a Jacobian contribution only when the matching Jacobian hook is defined.
Misspelling a residual hook is a compile error.

The template argument `Derived` can be used for implementing static polymorphism in a [CRTP-like](syntax/Kokkos/index.md#kokkos_crtp) fashion by statically casting `this` pointer to the derived type and directly calling the derived class methods using the cast pointer.

The indices can no longer be member variables due to the [parallelization](syntax/Kokkos/index.md#kokkos_functor), and therefore they are now passed as function arguments.
`datum` is a temporary object holding element-private data, and `_q_point` is available through `datum` as `datum.q_point(qp)`.
`_current_elem` does not have a direct replacement, as it is a pointer to the current libMesh element object that cannot be accessed on GPU.
However, `datum.elem()` returns an object containing the information of the current element, and it can be used to retrieve mesh data from the Kokkos mesh object available through `kokkosMesh()`.
Note that the mesh data available on GPU is currently very limited and will be continuously enhanced.
Variable and shape objects now use a function call syntax instead of the array indexing syntax and require `datum` as an additional argument to the indices.
Consequently, the following residual and Jacobian functions in `Diffusion`:

```cpp
Real
Diffusion::computeQpResidual()
{
  return _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
Diffusion::computeQpJacobian()
{
  return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
```

will look like the following in `KokkosDiffusion`, whose terms multiply the test function gradient:

```cpp
template <typename Derived>
KOKKOS_FUNCTION Real3
KokkosDiffusion::precomputeQpResidualGrad(const unsigned int qp, AssemblyDatum & datum) const
{
  return _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real3
KokkosDiffusion::precomputeQpJacobianGrad(const unsigned int j,
                                          const unsigned int qp,
                                          AssemblyDatum & datum) const
{
  return _grad_phi(datum, j, qp);
}
```

!alert note
[Every GPU function needs to be inlineable](syntax/Kokkos/index.md#kokkos_execution_space) and thus should be defined in headers.

`Moose::Kokkos::KernelValue` and `Moose::Kokkos::KernelGrad` are other names for `Moose::Kokkos::Kernel` that state in the class declaration which term a kernel contributes.
A kernel whose weak form has both terms, such as convection-diffusion, defines both families of hooks on a single class.
See the following source codes of `KokkosConvectionPrecompute` and `KokkosDiffusionPrecompute` for examples:

!listing test/include/kokkos/kernels/KokkosConvectionPrecompute.h id=kokkos-convection-precompute
         caption=The `KokkosConvectionPrecompute` header file.

!listing test/include/kokkos/kernels/KokkosDiffusionPrecompute.h id=kokkos-diffusion-precompute
         caption=The `KokkosDiffusionPrecompute` header file.

Kernels on vector variables derive from `Moose::Kokkos::VectorKernel` and use the same hooks with `Real3` and `Real33` coefficients, plus a `Curl` variant of each hook for weak forms containing $\nabla \times \boldsymbol{\psi}_i$.

## Time Derivative Kernels

[Like the original MOOSE](syntax/Kernels/index.md#time-derivative), you can create a time-derivative kernel by subclassing `Moose::Kokkos::TimeKernel`, which also provides `_u_dot` and `_du_dot_du`.
`Moose::Kokkos::TimeKernelValue` is another name for it.
In Kokkos-MOOSE, the dummy `_qp` indexing of the `du_dot_du` term was lifted.
The following shows the conversion of the example presented in the original page into the Kokkos version:

- For `computeQpResidual()` whose original code is:

```cpp
return _test[_i][_qp] * _u_dot[_qp];
```

the Kokkos version of `precomputeQpResidualValue()` will look like:

```cpp
return _u_dot(datum, qp);
```

- For `computeQpJacobian()` whose original code is:

```cpp
return _test[_i][_qp] * _phi[_j][_qp] * _du_dot_du[_qp];
```

the Kokkos version of `precomputeQpJacobianValue()` will look like:

```cpp
return _phi(datum, j, qp) * _du_dot_du;
```

See the following source codes of `KokkosTimeDerivative` for an example of a time-derivative kernel:

!listing framework/include/kokkos/kernels/KokkosTimeDerivative.h id=kokkos-time-derivative-header
         caption=The `KokkosTimeDerivative` header file.

!listing framework/src/kokkos/kernels/KokkosTimeDerivative.K id=kokkos-time-derivative-source language=cpp
         caption=The `KokkosTimeDerivative` source file.

## Automatic Differentiation id=kokkos_ad_kernel

Kokkos-MOOSE kernels also support [automatic differentiation (AD)](automatic_differentiation/index.md).
AD kernels derive from `Moose::Kokkos::ADKernel`, are registered with `registerKokkosADResidualObject()`, and define the same residual hooks as ordinary kernels, `precomputeQpResidualValue()` and `precomputeQpResidualGrad()`, returning dual numbers instead of reals.
The Jacobian is assembled from the derivatives of those dual numbers, so no Jacobian hooks are defined.

The derivatives are taken pointwise: their directions are the quadrature point quantities the kernel reads, one direction for each coupled value and three for each coupled gradient, rather than the degrees of freedom of the element.
The kernel evaluates its hooks once per quadrature point, and the base class contracts their derivatives with the shape functions of each coupled variable.
A kernel declares the number of directions it reads as `max_ad_slots`, and templates its hooks on the number of slots of the datum, which sizes its dual numbers at compile time:

```cpp
class KokkosADDiffusion : public Moose::Kokkos::ADKernel
{
public:
  /// The residual depends on grad u
  static constexpr unsigned int max_ad_slots = 3;

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal3<slots>
  precomputeQpResidualGrad(const unsigned int qp, Moose::Kokkos::ADAssemblyDatum<slots> & datum) const
  {
    return _grad_u(datum, qp);
  }

private:
  const Moose::Kokkos::ADVariableGradient _grad_u;
};
```

The accessors are created in the constructor with `adValue()`, `adGradient()`, and `adDot()` for the kernel variable, or `adCoupledValue()`, `adCoupledGradient()`, and `adCoupledDot()` for coupled variables, for example `_grad_u(adGradient())`.
During initial setup, each accessor of a nonlinear variable that the coupling matrix couples to the kernel variable receives its derivative directions; every other accessor, such as one reading an auxiliary variable, a constant, or a variable whose coupling the preconditioner omits, returns its value without derivatives.
The Jacobian therefore contains exactly the blocks the coupling matrix requests.
An object whose accessors receive more directions than its `max_ad_slots` errors during setup.

An AD kernel or integrated boundary condition with `use_displaced_mesh = true` is posed on the current configuration.
Its hooks are then instantiated with `max_ad_slots` plus the directions of the displacements, the value and reference gradient of each component, and the geometry of the current configuration, which the accessors and the base class compose from the dual deformation gradient, carries the derivatives with respect to the displacements.
The Jacobian with respect to the displacements is therefore assembled with no change to the kernel.

!syntax list /Kernels objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md
