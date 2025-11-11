# Kokkos Functions System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [Functions System](syntax/Functions/index.md) to understand the MOOSE function system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

!alert note
Kokkos-MOOSE functions do not support automatic differention yet.

A Kokkos-MOOSE function can be created by subclassing `Moose::Kokkos::FunctionBase` (not `Moose::Kokkos::Function`), which should be registered with `registerKokkosFunction()` instead of `registerMooseObject()`.
The signatures of the hook methods are defined as follows:

```cpp
KOKKOS_FUNCTION Real value(Real t, Real3 p) const;
KOKKOS_FUNCTION Real3 vectorValue(Real t, Real3 p) const;
KOKKOS_FUNCTION Real3 gradient(Real t, Real3 p) const;
KOKKOS_FUNCTION Real3 curl(Real t, Real3 p) const;
KOKKOS_FUNCTION Real div(Real t, Real3 p) const;
KOKKOS_FUNCTION Real timeDerivative(Real t, Real3 p) const;
KOKKOS_FUNCTION Real timeIntegral(Real t1, Real t2, Real3 p) const;
KOKKOS_FUNCTION Real integral() const;
KOKKOS_FUNCTION Real average() const;
```

As in other Kokkos-MOOSE objects, they should be defined in the derived class as +*inlined public*+ methods instead of virtual override.
It is not mandatory to define each hook method in the derived class, but any hook method that was not defined in the derived class should not be called.

See the following source codes of `KokkosPiecewiseConstant` for an example of a function:

!listing framework/include/kokkos/functions/KokkosPiecewiseConstant.h id=kokkos-piecewise-constant-header
         caption=The `KokkosPiecewiseConstant` header file.

!listing framework/src/kokkos/functions/KokkosPiecewiseConstant.K id=kokkos-piecewise-constant-source language=cpp
         caption=The `KokkosPiecewiseConstant` source file.

Functions can be acquired in your object by calling `getKokkosFunction<T>()`, where `T` should be your function type.
Namely, the actual type of the function should be known in advance.
This is the limitation of the current Kokkos-MOOSE function implementation which is being addressed, and more discussions can be found below.
It is recommended to store the acquired function in a `ReferenceWrapper` instance.
Otherwise, it should be stored as a copy, and it becomes your responsibility to maintain synchronization between the original function and the copy.

## Use of Dynamic Polymorphism

Unlike most of other Kokkos-MOOSE objects, functions are pluggable objects that are to be used in other objects.
Therefore, it is desired to use functions type-agnostically in your object so that any type of function can be plugged in.
Such design requires dynamic polymorphism, which is represented by virtual functions.
Although using virtual functions on GPU can sacrifice performance due to the inability of code inlining and vtable lookup and is still advised to avoid if possible, it can be useful for avoiding code duplications and improving code maintainability.

In fact, Kokkos-MOOSE already has the code path for acquiring functions as a type-agnostic abstract type and using virtual dispatch to evaluate actual functions.
There is another non-template API `getKokkosFunction()` which returns an abstract type `Moose::Kokkos::Function`.
It is not the base class of your function but a wrapper class that contains your function and provides shims to call the function methods.

However, that code path is currently blocked for GPU backends, and you will hit a runtime error when you attempt to use it.
While our target GPU backends (CUDA, HIP, and Intel SYCL) support virtual functions on GPU, it requires the relocatable device code (RDC) option to be enabled during compilation, which the current MOOSE build system is lacking.
Kokkos imposes a restriction to have a consistent RDC configuration across the whole software stack, but PETSc, from which MOOSE is currently acquring Kokkos, requires significant changes in its build system to enable RDC.
The non-templated `getKokkosFunction()` code path is hypothetically functional under the assumption that the MOOSE software stack can be built with the RDC option, but this hypothetical will only become a reality after a PETSc build system rework.
The PETSc and MOOSE team are looking into this issue, and it is expected to be resolved in the foreseeable future.

!syntax list /Functions objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md
