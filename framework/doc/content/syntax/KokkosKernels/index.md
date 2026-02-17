# Kokkos Kernels System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [Kernels System](syntax/Kernels/index.md) to understand the MOOSE kernel system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE.

!alert note
Kokkos-MOOSE kernels do not support coupling with scalar variables and automatic differention yet.

The Kokkos-MOOSE kernels are designed to resemble the original MOOSE kernels as much as possible for easier porting and adaptation.
However, some differences still exist due to the fundamentally different programming paradigm between CPU and GPU.
You can create your own kernel by subclassing `Moose::Kokkos::Kernel` as is done in the original MOOSE by inheriting `Kernel`.
However, your kernel should now be registered with either `registerKokkosKernel()` or `registerKokkosResidualObject()` instead of `registerMooseObject()`.
Also, the signatures of hook methods are different.
In the original MOOSE, the following virtual functions should or optionally have been overriden:

```cpp
virtual Real computeQpResidual() override;
virtual Real computeQpJacobian() override;
virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
```

Indices such as `_i`, `_j`, and `_qp` were made available in those functions as member variables.

In Kokkos-MOOSE, the corresponding hook methods are +*no longer virtual functions*+.
Instead, they are (re)defined in the derived class as +*inlined public*+ methods.
Their signatures look like the following:

```cpp
KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                       const unsigned int qp,
                                       AssemblyDatum & datum) const;
KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                       const unsigned int j,
                                       const unsigned int qp,
                                       AssemblyDatum & datum) const;
KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int i,
                                              const unsigned int j,
                                              const unsigned int jvar,
                                              const unsigned int qp,
                                              AssemblyDatum & datum) const;
```

Analogously to the original MOOSE, `computeQpResidual()` must be provided in the derived class, and the definition of `computeQpJacobian()` and `computeQpOffDiagJacobian()` are optional.
The optional methods have default definitions in the base class, and redefining them in the derived class hides the base class definitions.
Beware not to misspell the function names when redefining the optional methods, as they will be silently ignored rather than throwing a compile error.

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

will look like the following in `KokkosDiffusion`:

```cpp
KOKKOS_FUNCTION Real
KokkosDiffusion::computeQpResidual(const unsigned int i,
                                   const unsigned int qp,
                                   AssemblyDatum & datum) const
{
  return _grad_u(datum, qp) * _grad_test(datum, i, qp);
}

KOKKOS_FUNCTION Real
KokkosDiffusion::computeQpJacobian(const unsigned int i,
                                   const unsigned int j,
                                   const unsigned int qp,
                                   AssemblyDatum & datum) const
{
  return _grad_phi(datum, j, qp) * _grad_test(datum, i, qp);
}
```

See the following source codes of `KokkosCoupledForce` for another example of a kernel:

!listing framework/include/kokkos/kernels/KokkosCoupledForce.h id=kokkos-force-header
         caption=The `KokkosCoupledForce` header file.

!listing framework/src/kokkos/kernels/KokkosCoupledForce.K id=kokkos-force-source language=cpp
         caption=The `KokkosCoupledForce` source file.

!alert note
[Every GPU function needs to be inlineable](syntax/Kokkos/index.md#kokkos_execution_space) and thus should be defined in headers.

## Multi-variable Kernels

Unlike the original MOOSE kernels which can only operate on a single variable, Kokkos-MOOSE kernels can operate on multiple variables in parallel.
Instead of the `variable` parameter, you can specify the `variables` parameter with arbitrarily many variables you want the kernel to operate on.
In the code, you can obtain which component of the variables you are currently operating on using `datum.comp()`.
This provides additional parallelism over the variables and potentially enhances performance by providing higher chance of memory coalescing.
The restrictions are that all variables should reside in the same system and have the same finite element shape function family and order.
Also, all variables should have the same number of off-diagonal coupling.

Multi-variable kernels can be useful in certain applications where the same kernel applies to multiple variables but using array variables is not desired.
One example is neutronics, where there are multiple flux variables representing different energy groups that use the same kernel.
It is desired to keep the flux variable of each group as separate standard variables, so that they can be individually coupled to other objects.
Using an array variable here makes it cumbersome to access the flux solution of each group.

!alert note
All other Kokkos-MOOSE residual objects such as [BCs](syntax/KokkosBCs/index.md) and [NodalKernels](syntax/KokkosNodalKernels/index.md) also support operating on multiple variables with the same restrictions.

## Optimized Kernel Objects

[Similarly to the original MOOSE](syntax/Kernels/index.md#optimized), Kokkos-MOOSE provides `Moose::Kokkos::KernelValue` and `Moose::Kokkos::KernelGrad` for creating an optimized kernel by factoring out test functions in residual and Jacobian calculations.
However, instead of `precomputeQpResidual()` and `precomputeQpJacobian()`, you should still define `computeQpResidual()` and `computeQpJacobian()` but with different signatures from those in `Moose::Kokkos::Kernel`.
The signatures of the hook methods are as follows:

- For `Moose::Kokkos::KernelValue`,

```cpp
KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp,
                                       AssemblyDatum & datum) const;
KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int j,
                                       const unsigned int qp,
                                       AssemblyDatum & datum) const;
```

- For `Moose::Kokkos::KernelGrad`,

```cpp
KOKKOS_FUNCTION Real3 computeQpResidual(const unsigned int qp,
                                        AssemblyDatum & datum) const;
KOKKOS_FUNCTION Real3 computeQpJacobian(const unsigned int j,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const;
```

See the following source codes of `KokkosConvectionPrecompute` and `KokkosDiffusionPrecompute` for examples of optimized kernels:

!listing test/include/kokkos/kernels/KokkosConvectionPrecompute.h id=kokkos-convection-precompute
         caption=The `KokkosConvectionPrecompute` header file.

!listing test/include/kokkos/kernels/KokkosDiffusionPrecompute.h id=kokkos-diffusion-precompute
         caption=The `KokkosDiffusionPrecompute` source file.

## Time Derivative Kernels

[Like the original MOOSE](syntax/Kernels/index.md#time-derivative), you can create a time-derivative kernel by subclassing `Moose::Kokkos::TimeKernel`.
In Kokkos-MOOSE, the dummy `_qp` indexing of the `du_dot_du` term was lifted.
Instead, the current variable component index `datum.comp()` should be passed as an argument.
The following shows the conversion of the example presented in the original page into the Kokkos version:

- For `computeQpResidual()` whose original code is:

```cpp
return _test[_i][_qp] * _u_dot[_qp];
```

the Kokkos version will look like:

```cpp
return _test(datum, i, qp) * _u_dot(datum, qp);
```

- For `computeQpJacobian()` whose original code is:

```cpp
return _test[_i][_qp] * _phi[_j][_qp] * _du_dot_du[_qp];
```

the Kokkos version will look like:

```cpp
return _test(datum, i, qp) * _phi(datum, j, qp) * _du_dot_du[datum.comp()];
```

See the following source codes of `KokkosCoupledTimeDerivative` for an example of a time-derivative kernel:

!listing framework/include/kokkos/kernels/KokkosCoupledTimeDerivative.h id=kokkos-time-derivative-header
         caption=The `KokkosCoupledTimeDerivative` header file.

!listing framework/src/kokkos/kernels/KokkosCoupledTimeDerivative.K id=kokkos-time-derivative-source language=cpp
         caption=The `KokkosCoupledTimeDerivative` source file.

!syntax list /Kernels objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md
