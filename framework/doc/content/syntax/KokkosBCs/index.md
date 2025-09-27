# Kokkos BCs System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [BCs System](syntax/BCs/index.md) to understand the MOOSE boundary condition system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

!alert note
Kokkos-MOOSE boundary conditions do not support automatic differention yet.

The basic design pattern of Kokkos-MOOSE kernels described in [Kokkos Kernels System](syntax/Kokkos/index.md) applies to the boundary conditions as well.
You can create your own integrated and nodal boundary conditions by subclassing `Moose::Kokkos::IntegratedBC` and `Moose::Kokkos::NodalBC`, respectively, and following the same pattern with kernels including registering your boundary conditions with `registerKokkosResidualObject()`.
Especially, integrated boundary conditions have identical interfaces with kernels, so they will not be explained here in detail.
See the following source codes of `KokkosCoupledVarNeumannBC` for an example of an integrated boundary condition:

!listing framework/include/kokkos/bcs/KokkosCoupledVarNeumannBC.h id=kokkos-neumann-header
         caption=The `KokkosCoupledVarNeumannBC` header file.

!listing framework/src/kokkos/bcs/KokkosCoupledVarNeumannBC.K id=kokkos-neumann-source language=cpp
         caption=The `KokkosCoupledVarNeumannBC` source file.

On the other hand, nodal boundary conditions have slightly different interfaces.
The hook methods for a nodal boundary condition have the following signatures:

```cpp
KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, ResidualDatum & datum) const;
KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int qp, ResidualDatum & datum) const;
KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int jvar,
                                              const unsigned int qp,
                                              ResidualDatum & datum) const;
```

The test and trial function indices, `i` and `j`, are no longer passed as arguments.
`qp` corresponds to the dummy `_qp` index in the original MOOSE and is always zero.
To keep the consistency between interfaces, however, it is still passed as an argument and used for getting solution values.
`_current_node`, which is a pointer to the current libMesh node object, does not have a direct replacement.
Instead, the node index can be queried by `datum.node()` and used to retrieve mesh data from the Kokkos mesh object.
The node coordinate can also be obtained by `datum.q_point(qp)`.
As a result, the following residual function in `DirichletBCBase`:

```cpp
Real
DirichletBCBase::computeQpResidual()
{
  return _u[_qp] - computeQpValue();
}
```

becomes the following in `Moose::Kokkos::DirichletBCBase`:

```cpp
template <typename Derived>
KOKKOS_FUNCTION Real
DirichletBCBase<Derived>::computeQpResidual(const unsigned int qp, ResidualDatum & datum) const
{
  auto bc = static_cast<const Derived *>(this);

  return _u(datum, qp) - bc->computeValue(qp, datum);
}
```

Also note here the static implementation of `computeValue` using the [Curiously Recurring Template Pattern (CRTP)](syntax/Kokkos/index.md#kokkos_crtp) which is originally a virtual function.

See the following source codes of `KokkosMatchedValueBC` for another example of a nodal boundary condition:

!listing framework/include/kokkos/bcs/KokkosMatchedValueBC.h id=kokkos-matched-header
         caption=The `KokkosMatchedValueBC` header file.

!listing framework/src/kokkos/bcs/KokkosMatchedValueBC.K id=kokkos-matched-source language=cpp
         caption=The `KokkosMatchedValueBC` source file.

!syntax list /KokkosBCs objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md
