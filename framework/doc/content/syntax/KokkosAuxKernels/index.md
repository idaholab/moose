# Kokkos AuxKernels System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [AuxKernels System](syntax/AuxKernels/index.md) to understand the MOOSE auxiliary kernel system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

!alert note
Kokkos-MOOSE auxiliary kernels do not support boundary-restricted elemental kernels yet.

The Kokkos-MOOSE auxiliary kernels can be created by inheriting `Moose::Kokkos::AuxKernel` and registering them with `registerKokkosAuxKernel()`.
There is no vector or array version at this moment.
The signature of the `computeValue()` hook method is now as follows:

```cpp
KOKKOS_FUNCTION Real computeValue(const unsigned int qp, ResidualDatum & datum) const;
```

To reiterate, the hook method no longer uses virtual dispatch.
Instead, it should be statically defined in the derived class as +*inlined public*+ method.

Like the original MOOSE, the single interface is used for both elemental and nodal kernels, and whether the object is elemental or nodal can be queried with the `isNodal()` method on both CPU and GPU.
Depending on whether the kernel is nodal or elemental, some data that can be queried through `datum` may be invalid.
For instance, `datum.node()` will throw an invalid index in elemental kernels, and vice versa.
Therefore, if you need to treat elemental and nodal variables separately, it is important to properly separate their code paths.
For nodal kernels, the index `qp` is simply zero but still passed as an argument to have a unified interface, as explained in [nodal boundary conditions](syntax/KokkosBCs/index.md).

The following example shows the implementation of `KokkosVariableTimeIntegrationAux`:

!listing framework/include/kokkos/auxkernels/KokkosVariableTimeIntegrationAux.h id=kokkos-time-integration-aux-header
         caption=The `KokkosVariableTimeIntegrationAux` header file.

!listing framework/src/kokkos/auxkernels/KokkosVariableTimeIntegrationAux.K id=kokkos-time-integration-aux-source language=cpp
         caption=The `KokkosVariableTimeIntegrationAux` source file.

## Advanced Topics

The computational loops are defined in `computeElementInternal()` and `computeNodeInternal()` for elemental and nodal variables, respectively, and they call the hook method `computeValue()`.
Depending on the type of operations, however, you may be able to write more efficient kernels by customizing the loops directly.
For example, what `KokkosCopyValueAux` does is simply copy one auxiliary variable to another, and the shape function family and order are always identical between the two variables.
In this case, you can directly loop over DOFs instead of quadrature points.
See the following source code of `KokkosCopyValueAux`, where `computeElementInternal()` and `computeNodeInternal()` are redefined instead of `computeValue()`:

!listing framework/include/kokkos/auxkernels/KokkosCopyValueAux.h id=kokkos-copy-aux-header
         caption=The `KokkosCopyValueAux` header file.

!listing framework/src/kokkos/auxkernels/KokkosCopyValueAux.K id=kokkos-copy-aux-source language=cpp
         caption=The `KokkosCopyValueAux` source file.

!syntax list /KokkosAuxKernels objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md
