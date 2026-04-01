# Kokkos UserObjects System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [UserObjects System](syntax/UserObjects/index.md) to understand the MOOSE user object system,
- [Postprocessors System](syntax/Postprocessors/index.md) to understand the MOOSE postprocessor system,
- [VectorPostprocessors System](syntax/VectorPostprocessors/index.md) to understand the MOOSE vector postprocessor system,
- [Reporters System](syntax/Reporters/index.md) to understand the MOOSE reporter system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

Currently, the following types of user objects (and their derivatives such as postprocessors) are supported in Kokkos-MOOSE, which all should be registered with `registerKokkosUserObject()`:

- `Moose::Kokkos::ElementUserObject`
- `Moose::Kokkos::SideUserObject`
- `Moose::Kokkos::NodalUserObject`

The hook method `execute()` is now defined as an +*inlined public*+ method with the following signature:

```cpp
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

## User-defined APIs and Virtual Functions

While user objects are intended to embrace user-defined APIs, Kokkos-MOOSE user objects currently require GPU APIs to not rely on virtual dispatch.
Namely, you should always retrieve your user objects in their concrete types if you intend to use your own GPU APIs.
Using virtual functions on GPU has two prerequisites: enabling the relocatable device code (RDC) option and constructing objects on GPU.
The RDC is option is currently disabled in Kokkos-MOOSE due to the restrictions imposed by upstream packages (see [the discussions on this page](syntax/KokkosFunctions/index.md#kokkos_rdc)), and its resolution is being actively worked on.
Even with the RDC option, however, the object vtables are populated with CPU function pointers as all objects in MOOSE are constructed on CPU.
As a result, you still cannot call virtual functions of your user objects on GPU unless you directly construct them on GPU (see [this page](syntax/Kokkos/index.md#kokkos_crtp)).

In order to realize virtual dispatch with your user objects, therefore, you need to implement a wrapper with virtual functions that can be easily constructed on GPU, and call your own APIs through the wrapper.
This wrapper will hold the GPU copy of your user object in its concrete type and the virtual functions that call the corresponding user object functions statically.
This approach is implemented in `Moose::Kokkos::Function` using a registry design pattern and can be found across framework source files such as [KokkosFunctionWrapper.h](include/kokkos/functions/KokkosFunctionWrapper.h) and [KokkosFunction.h](include/kokkos/functions/KokkosFunction.h), but it requires a deep understanding of dynamic polymorphism and GPU backends.
Therefore, we plan to explore developing base classes for the wrapper that the users can easily derive from and providing programming guidelines, once the RDC option is in place.

## Reducers id=reducers

`Postprocessor`, `VectorPostprocessor`, and `Reporter` are common in that they derive from user objects and perform aggregate calculations.
In Kokkos-MOOSE, they are implemented under a common concept called `Reducer` and share the same GPU interface, and each of them has the identical CPU interface with its corresponding original MOOSE object.
Each reducer should be registered with either `registerKokkosReducer()` or the alias for each type (`registerKokkosPostprocessor()`, `registerKokkosVectorPostprocessor()`, `registerKokkosReporter()`).

The aggregate calculations, also known as reduction operations, are not trivial on GPU which performs a massively parallel computation, as the data race should be carefully managed.
Therefore, you cannot directly perform reduction operations on your own variables.
Instead, the redcution operations should be performed on a preallocated buffer defined by the reducer.
Every reducer should allocate the buffer with the desired size by calling `allocateReductionBuffer()` prior to the calculation, which can be done either in the constructor or in the `initialize()` hook.
It allocates `_reduction_buffer`, which is a one-dimensional `Kokkos::View` defined in the CPU space.
And the `execute()` hook method now receives an additional argument `result` that points to a buffer where you need to perform your reduction operations.
This buffer has the same size with `_reduction_buffer`, but it is a buffer internally defined by Kokkos and is different from `_reduction_buffer`:

```cpp
KOKKOS_FUNCTION void execute(Datum & datum, Real * result) const;
```

A reducer also requires two more hook methods to be defined in your derived object, which are `join()` and `init()`.
They have the following signatures:

```cpp
KOKKOS_FUNCTION void join(DefaultLoop, Real * result, const Real * source) const;
KOKKOS_FUNCTION void init(DefaultLoop, Real * result) const;
```

`join()` can be considered as a replacement of `threadJoin()` in the original MOOSE objects.
It combines the values of `source` into `result` and should typically implement the same reduction operation with `execute()`.
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

See the following source codes of `KokkosIntegralPostprocessor` for an example of a postprocessor (note that the example is [defining the shim](syntax/Kokkos/index.md#kokkos_shim) of `execute()` instead and changing the hook method to `computeQpIntegral()`):

!listing framework/include/kokkos/postprocessors/KokkosIntegralPostprocessor.h id=kokkos-integral-postprocessor-header
         caption=The `KokkosIntegralPostprocessor` header file.

!listing framework/src/kokkos/postprocessors/KokkosIntegralPostprocessor.K id=kokkos-integral-postprocessor-source language=cpp
         caption=The `KokkosIntegralPostprocessor` source file.

!alert note
Kokkos-MOOSE vector postprocessors and reporters are comming soon.

!alert note
The reporter values defined by Kokkos-MOOSE postprocessors, vector postprocessors, and reporters are stored in the same database with the original MOOSE objects, so they can be retrieved through ordinary interfaces and their names cannot overlap.

### Performance Considerations

The [Kokkos Reducer concept](https://kokkos.org/kokkos-core-wiki/API/core/builtinreducers/ReducerConcept.html) leveraged to implement Kokkos-MOOSE reducers is suitable for small arrays and "dense" reduction operations.
Assume you are reducing a large array, where a single call to `execute()` only accumulates values to one or a few entries of the array at most.
This can be considered as a "sparse" reduction operation and is not suitable for a reducer.
A massively-parallel reduction is implemented by many partial reductions where many temporary buffers are internally created and initialized.
Even though most of the entries of a temporary buffer are unused in each partial reduction, it still has to initialize and join all the entries.
As a result, the cost of initialization and join can overwhelm the gain from parallelization.
In addition, the temporary buffers are typically allocated in user-managed caches with limited sizes like shared memory in CUDA, so small arrays are desired.
Such type of reduction operation is therefore better be implemented using [atomic operations](https://kokkos.org/kokkos-core-wiki/ProgrammingGuide/Atomic-Operations.html) in a regular parallel loop by deriving a user object instead.

!syntax list /UserObjects objects=True actions=False subsystems=False

!syntax list /Postprocessors objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md
