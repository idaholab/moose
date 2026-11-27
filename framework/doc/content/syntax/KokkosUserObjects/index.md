# Kokkos UserObjects System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [UserObjects System](syntax/UserObjects/index.md) to understand the MOOSE user object system,
- [Postprocessors System](syntax/Postprocessors/index.md) to understand the MOOSE postprocessor system,
- [VectorPostprocessors System](syntax/VectorPostprocessors/index.md) to understand the MOOSE vector postprocessor system,
- [Reporters System](syntax/Reporters/index.md) to understand the MOOSE reporter system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

Currently, the following types of user objects are supported in Kokkos-MOOSE including their derivatives (postprocessors, vector postprocessors, and reporters), which all should be registered with `registerKokkosUserObject()`:

- `Moose::Kokkos::ElementUserObject`
- `Moose::Kokkos::SideUserObject`
- `Moose::Kokkos::NodalUserObject`

The hook method `execute()` is now defined as an +*inlined public*+ method with the following signature:

```cpp
template <typename Derived>
KOKKOS_FUNCTION void execute(Datum & datum) const;
```

For other CPU APIs, Kokkos-MOOSE user objects share the same interface with the original MOOSE objects except `threadJoin()`, which is undefined in Kokkos-MOOSE user objects as they have separate parallel loops dispatched by Kokkos.

!alert note
The dependencies between Kokkos-MOOSE user objects and the original MOOSE user objects are not automatically respected, and it is highly discouraged to have dependencies between them.
However, if you need objects from both of those categories to be executed in a specific order, you can manually specify the `execution_order_group` parameter.
A negative group may be specified to execute before the default group (0).
For the same group, Kokkos-MOOSE user objects are always executed prior to the original MOOSE user objects.
For this reason, the Kokkos version of general user object (`Moose::Kokkos::GeneralUserObject`) is still provided in case there is a general user object having dependencies with other Kokkos-MOOSE user objects, although it does not execute any parallel loop.
A Kokkos-MOOSE general user object should be registered with the standard `registerMooseObject()` macro.

!alert note
Different types of Kokkos-MOOSE user objects are executed without any predefined order (dependencies are still respected).

## Reducers id=reducers

Postprocessors, vector postprocessors, and reporters often perform calculations that aggregate data.
Such aggregation calculation is also known as reduction operation.
If your calculation is considered more suitable for a reduction operation than an ordinary parallel operation, you can make your user object a `Reducer` object by defining different hook methods, which will use a different parallelization scheme from regular parallel objects.

Reduction operations are not trivial on GPU due to its massively parallel nature that requires the data race to be carefully managed.
Therefore, you cannot directly perform reduction operations on your own variables.
Instead, the reduction operations should be performed on a preallocated buffer defined by the reducer.
Every reducer should allocate the buffer with the desired size by calling `allocateReductionBuffer()` prior to the calculation, which can be done either in the constructor or in the `initialize()` hook.
It allocates `_reduction_buffer`, which is a one-dimensional `Kokkos::View` defined in the CPU space.
The hook method is now named as `reduce()` and receives an additional argument `result` that points to a buffer where you need to perform your reduction operations.
This buffer has the same size with `_reduction_buffer`, but it is a buffer internally defined by Kokkos and is different from `_reduction_buffer`:

```cpp
template <typename Derived>
KOKKOS_FUNCTION void reduce(Datum & datum, Real * result) const;
```

A reducer also requires two more hook methods to be defined in your derived object, which are `join()` and `init()`.
They have the following signatures:

```cpp
template <typename Derived>
KOKKOS_FUNCTION void join(Real * result, const Real * source) const;
template <typename Derived>
KOKKOS_FUNCTION void init(Real * result) const;
```

`join()` can be considered as a replacement of `threadJoin()` in the original MOOSE objects.
It combines the values of `source` into `result` and should typically implement the same reduction operation with `reduce()`.
`init()` can be considered as a replacement of `initialize()` in the original MOOSE objects where you used to initialize your reduction variables.
It initializes the Kokkos internal buffer on GPU.
Note that initializing `_reduction_buffer` has no effect on GPU and thus is not required.
Once the calculation is complete, the results are stored in `_reduction_buffer`.
You can process the values stored in `_reduction_buffer` in the `finalize()` or `getValue()` hook to compute final values.

It is important to note that the buffer is always provided as the `Real` type.
If you need to implement a reduction operation for a data type other than `Real`, you need to do type casting or [bit casting](https://kokkos.org/kokkos-core-wiki/API/core/numerics/bit-manipulation.html) of the given buffer to the desired type.
Typically, the data types used for reductions other than `Real` are boolean and integer.
Boolean operations can be performed with `Real`, and the integers in the range $[−2^{53}, 2^{53}]$ can be exactly represented by `Real`.
Therefore, the need for a bit-level manipulation will be highly unlikely.

See the following source codes of `KokkosIntegralPostprocessor` for an example of a postprocessor:

!listing framework/include/kokkos/postprocessors/KokkosIntegralPostprocessor.h id=kokkos-integral-postprocessor-header
         caption=The `KokkosIntegralPostprocessor` header file.

!listing framework/src/kokkos/postprocessors/KokkosIntegralPostprocessor.K id=kokkos-integral-postprocessor-source language=cpp
         caption=The `KokkosIntegralPostprocessor` source file.

!alert note
The reporter values defined by Kokkos-MOOSE postprocessors, vector postprocessors, and reporters are stored in the same database with the original MOOSE objects, so they can be retrieved through the existing interfaces and their names cannot overlap.

### Performance Considerations id=reducer_atomic

The [Kokkos Reducer concept](https://kokkos.org/kokkos-core-wiki/API/core/builtinreducers/ReducerConcept.html) leveraged to implement Kokkos-MOOSE reducers is suitable for small arrays and "dense" reduction operations.
Assume you are reducing a large array, where a single call to `reduce()` only accumulates values to one or a few entries of the array at most.
This can be considered as a "sparse" reduction operation and is not suitable for a reducer.
A massively-parallel reduction is implemented by many partial reductions where many temporary buffers are internally created and initialized.
Even though most of the entries of a temporary buffer are unused in each partial reduction, it still has to initialize and join all the entries.
As a result, the cost of initialization and join can overwhelm the gain from parallelization.
In addition, the temporary buffers are typically allocated in configurable caches with limited sizes like shared memory in CUDA, so small arrays are desired.
If the array being reduced is too large, you will likely get an error like `Kokkos::Impl::ParallelReduce<Cuda> requested too much L0 scratch memory`.
Such type of reduction operation is therefore better be implemented using [atomic operations](https://kokkos.org/kokkos-core-wiki/ProgrammingGuide/Atomic-Operations.html) in an ordinary parallel loop instead of a reduction loop.
See the following example of `KokkosExtraIDIntegralVectorPostprocessor`, which implements both atomic addition and reduction algorithms and provides users an option to choose the calculation mode:

!listing framework/include/kokkos/vectorpostprocessors/KokkosExtraIDIntegralVectorPostprocessor.h id=kokkos-extra-id-vpp-header
         caption=The `KokkosExtraIDIntegralVectorPostprocessor` header file.

!listing framework/src/kokkos/vectorpostprocessors/KokkosExtraIDIntegralVectorPostprocessor.K id=kokkos-extra-id-vpp-source language=cpp
         caption=The `KokkosExtraIDIntegralVectorPostprocessor` source file.

## Virtual User Objects id=virtual_user_objects

Kokkos-MOOSE user objects can provide dynamic polymorphism through `Moose::Kokkos::VirtualUserObject<Base>`.
This allows a consuming Kokkos object to depend on a common user object base while selecting the concrete user object type from the input file.
Unlike ordinary C++ virtual functions, the user object hook methods remain non-virtual methods on the host-constructed MOOSE objects.
Kokkos-MOOSE generates the Kokkos virtual interface from the hook names registered for the base class.

Virtual user objects are useful when several user objects implement the same application-specific API but the consuming object should not know their concrete types.
Concrete typed access remains preferable when runtime polymorphism is unnecessary because it permits direct calls and compiler inlining.

!alert warning
Virtual Kokkos user objects are currently supported only by CPU execution backends.
Calling `getVirtualKokkosUserObject()` in a build configured with GPU backends produces an error.
Also see [this page](syntax/KokkosFunctions/index.md#kokkos_rdc).

### Defining the Virtual Base

The base class declares each hook as a public, `const`, `KOKKOS_FUNCTION` method.
The declarations do not use the C++ `virtual` keyword and do not require definitions.
After the complete class definition, `registerVirtualKokkosUserObjectBase()` registers the base and lists the hook names.
The macro infers each complete signature from its declaration, so hooks may use different return types and argument lists.

The following test base registers the `value()` and `combine()` hooks:

!listing test/include/kokkos/userobjects/KokkosPolymorphicUserObjectBase.h
         id=kokkos-virtual-user-object-base
         caption=A virtual Kokkos user object base with two user-defined hooks.

The base registration must remain in the header after the class definition.
This makes the hook interface visible wherever a concrete implementation or a `VirtualUserObject` is instantiated.
Hook names must be unique within the base because overloaded hook names cannot be identified by the registration macro.

### Defining Concrete Implementations

Each concrete user object derives from the registered base and defines every registered hook as a public, `const`, `KOKKOS_FUNCTION` method with exactly the signature declared by the base.
The methods are ordinary non-virtual methods.
The concrete type is registered with both its normal MOOSE registration macro and `registerVirtualKokkosUserObject(Derived, Base)` in its source file.
The first argument identifies the concrete derived type and the second argument explicitly identifies its registered virtual base.
This explicit relationship allows an intermediate class to be a concrete implementation of one virtual base while also serving as a virtual base for further derived classes.
A concrete type can implement multiple registered bases in its inheritance chain by invoking `registerVirtualKokkosUserObject(Derived, Base)` once for each base.
The registry uses both the concrete MOOSE type name and the requested base type, so each registration remains independent.

For example, `KokkosQuadraticUserObject` derives from the intermediate `KokkosAffineUserObject`, which derives from `KokkosPolymorphicUserObjectBase`.
The quadratic object is registered with both bases:

```cpp
registerVirtualKokkosUserObject(KokkosQuadraticUserObject, KokkosPolymorphicUserObjectBase);
registerVirtualKokkosUserObject(KokkosQuadraticUserObject, KokkosAffineUserObject);
```

The affine implementation supplies both hooks as follows:

!listing test/include/kokkos/userobjects/KokkosAffineUserObject.h
         id=kokkos-virtual-user-object-affine-header
         caption=An affine implementation of the virtual Kokkos user object interface.

!listing test/src/kokkos/userobjects/KokkosAffineUserObject.K
         id=kokkos-virtual-user-object-affine-source language=cpp
         caption=Registration and construction of the affine implementation.

The same base can have any number of registered concrete implementations.
For example, the quadratic implementation provides different behavior through the same hook signatures:

!listing test/include/kokkos/userobjects/KokkosQuadraticUserObject.h
         id=kokkos-virtual-user-object-quadratic-header
         caption=A quadratic implementation of the same virtual Kokkos user object interface.

!listing test/src/kokkos/userobjects/KokkosQuadraticUserObject.K
         id=kokkos-virtual-user-object-quadratic-source language=cpp
         caption=Registration and construction of the quadratic implementation.

### Acquiring and Calling a Virtual User Object

A consuming Kokkos-MOOSE object stores the user object as `Moose::Kokkos::VirtualUserObject<Base>` and acquires it with `getVirtualKokkosUserObject<Base>()`.
The returned object exposes the registered hook names directly, so device code calls `value()` and `combine()` rather than a generic call operator.

The following auxiliary kernel stores two virtual user objects and calls both registered hooks from `computeValue()`:

!listing test/include/kokkos/auxkernels/KokkosPolymorphicUserObjectAux.h
         id=kokkos-virtual-user-object-aux-header
         caption=A Kokkos auxiliary kernel that calls virtual user object hooks during Kokkos execution.

!listing test/src/kokkos/auxkernels/KokkosPolymorphicUserObjectAux.K
         id=kokkos-virtual-user-object-aux-source language=cpp
         caption=Acquiring virtual user objects through user object parameters.

The input file can then select different concrete types for each user object parameter without changing the consuming auxiliary kernel:

!listing test/tests/kokkos/userobjects/polymorphic_wrapper/polymorphic_wrapper.i
         id=kokkos-virtual-user-object-input
         caption=Selecting affine and quadratic implementations of one virtual user object base.

!syntax list /UserObjects objects=True actions=False subsystems=False

!syntax list /Postprocessors objects=True actions=False subsystems=False

!syntax list /VectorPostprocessors objects=True actions=False subsystems=False

!syntax list /Reporters objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md
