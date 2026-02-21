# Getting Started with Kokkos-MOOSE

!if! function=hasCapability('kokkos')

## Introduction

Kokkos-MOOSE is a part of the MOOSE framework which utilizes [Kokkos](https://kokkos.org/) to port the foundational components of MOOSE to GPU while retaining the original design as much as possible.
Kokkos provides a unified programming model for different GPU architectures by abstracting hardware-specific details, enabling developers to write a single codebase that can be compiled and executed on various platforms without the need for extensive code modification.
It is challenging to maintain portability across different architectures in today’s rapidly evolving hardware landscape, and the portability of Kokkos makes it suitable as a programming model for a framework like MOOSE which should serve diverse userbase.
While Kokkos-MOOSE has been only tested with NVIDIA GPUs thus far, the support for AMD and Intel GPUs will be added in the future as we diversify our testing hardware environment at Idaho National Laboratory (INL).

## Installing Kokkos-MOOSE

Installation instructions for Kokkos-MOOSE can be found in [this page](getting_started/installation/install_kokkos.md optional=True).

## Developing with Kokkos-MOOSE

Programming for GPU is fundamentally different from the ordinary CPU programming in various aspects.
Here we provide instructions on some programming practices to write an efficient and robust GPU code in Kokkos-MOOSE.

### Separate Memory Space

Except for a very few rare cases that provide physically unified memory space for CPU and GPU (such as AMD MI300A), most GPUs have separate memory spaces from their conuterpart CPUs.
Therefore, the you need to take a special care to properly identify which data are accessible and not accessible on either CPU or GPU and whether the data on CPU and GPU are properly synchronized.
Standard containers such as `std::vector`, `std::set`, `std::map` and others are not usable on GPU, and managed pointers are also inaccessible on GPU.
Basically, it is safe to assume that any dynamically-allocated data on CPU cannot be accessed on GPU.
Therefore, we provide alternative data containers to be used on GPU: `Moose::Kokkos::Array`, `Moose::Kokkos::JaggedArray`, and `Moose::Kokkos::Map`.

`Moose::Kokkos::Array` is a template class designed to hold arbitrary type of data.
It receives up to four template arguments: data type, dimension, index type, and layout type.
It supports multi-dimensional indexing, and up to five-dimensional arrays are supported.
The dimension can either be specified through the second template argument with the default being one-dimension or using type aliases: for instance, a three-dimensional array of type `double` can be declared either by `Array<double, 3>` or `Array3D<double>`.
The entries of an array can be accessed with either `operator()` with multi-dimensional indices or `operator[]` with a flattened, dimensionless index, where the flattening follows a layout in which the innermost dimension varies the fastest.
The index type template argument is set to 8-byte integer by default to accomodate large arrays.
However, 8-byte integer computation is significantly more expensive than 4-byte integer computation.
If your array size is small enough, consider using 4-byte indices to optimize index calculations.
If having the outermost dimension run the fastest is desired for multi-dimensional arrays, the fourth layout template argument can be optionally set to `Moose::Kokkos::LayoutType::RIGHT` (default is `LEFT`).
They automatically return either CPU or GPU data depending on where they are being accessed.
Arrays can be allocated through the following APIs: `create()`, `createHost()`, and `createDevice()`.
`create()` allocates memories on both CPU and GPU, while `createHost()` or `createDevice()` only allocates memory on either CPU or GPU.
It is important to note that if the creation APIs are called for an initialized array, the original array will be destroyed and a new array will be created.
Namely, you cannot allocate data on both CPU and GPU by calling `createHost()` and `createDevice()` separately.
Calling the creation APIs for a shallow copy of an array will disassociate it with the original array.

Instead of allocating memory, an array can also wrap an external pointer.
Instead of calling `create()`, you can call `init()` which only initializes dimensional information without allocating data, and then call `aliasHost()` or `aliasDevice()` to let CPU or GPU pointer wrap an external pointer.
You can also combine `createHost()` or `createDevice()` along with `aliasHost()` or `aliasDevice()` to allocate data on one side and alias data on the other side.
In this case, `createHost()` or `createDevice()` should be called first.
The following example shows wrapping a CPU PETSc vector and creating its GPU clone with an array:

```cpp
PetscScalar * petsc_ptr;
PetscInt petsc_size;
VecGetArray(petsc_vector, &petsc_ptr);
VecGetLocalSize(petsc_vector, &petsc_size);

Array<PetscScalar> vector;

vector.createDevice(petsc_size);
vector.aliasHost(petsc_ptr);
vector.copyToDevice();
```

If the data type is not default-constructable, `create()` will only allocate a raw block of uninitialized memory using `malloc()`.
It is your responsibility to loop over the array and perform placement new to properly construct each entry.
For example:

```cpp
Array<NotDefaultConstructable> data;

data.create(n);

for (auto & datum : data)
  new (&datum) NotDefaultConstructable(...);
```

!alert note
The GPU memory is always uninitialized upon allocation.

Because the memory spaces are separate between CPU and GPU, there is no automatic synchronzation of data.
Therefore, data should be explicitly copied between them when one side is updated.
Arrays provide three APIs for this purpose: `copyToHost()`, `copyToDevice()`, and `copyToDeviceNested()`.
`copyToHost()` and `copyToDevice()` copies data from GPU to CPU and CPU to GPU, respectively, as their names imply.
`copyToDeviceNested()` is a shortcut for copying data from CPU to GPU for a nested array like `Array<Array<...>>`, and it calls `copyToDevice()` for all the inner arrays and itself in the correct order.
When creating a nested array, it is important to note that the arrays should be copied from the innermost to the outermost in order:

```cpp
Array<Array<Array<Real>>> data;

data.create(n1);

for (unsigned int i = 0; i < n1; ++i)
{
  data[i].create(n2);

  for (unsigned int j = 0; j < n2; ++j)
  {
    data[i][j].create(n3);
    ... // Populating data
    data[i][j].copyToDevice();
  }

  data[i].copyToDevice();
}

data.copyToDevice();
```

The reason `copyToDevice()` should also be called for the outer arrays is because the pointers held by the inner arrays changed by memory allocation.
However, this nested call can be replaced by a single call to `copyToDeviceNested()` for the outermost array:

```cpp
Array<Array<Array<Real>>> data;

data.create(n1);

for (unsigned int i = 0; i < n1; ++i)
{
  data[i].create(n2);

  for (unsigned int j = 0; j < n2; ++j)
  {
    data[i][j].create(n3);
    ... // Populating data
  }
}

data.copyToDeviceNested();
```

!listing framework/include/kokkos/base/KokkosArray.h
         id=kokkos_array_source
         caption=The `Moose::Kokkos::Array` source code.

`Moose::Kokkos::JaggedArray` is a special array object that aids in treating jagged arrays conveniently and efficiently by using a sequential data storage while still providing multi-dimensional indexing.
Jagged arrays appear commonly in many applications, but expressing jagged arrays as nested arrays like `Array<Array<...>>` can be inefficient on GPU because of memory fragmentation.
Therefore, `Moose::Kokkos::JaggedArray` stores the data of a jagged array sequentially and defines dope vectors internally to identify the location and size of each inner array.
It is divided into inner and outer arrays.
The outer array is the regular part of a jagged array.
Each entry of the outer array is the inner array, whose size can vary with each other.
As a result, it is defined with up to five template arguments: the data type, inner array dimension size, outer array dimension size, outer array index type (defaults to 8-byte integer; inner arrays always use 4-byte integer), and inner array layout type (defaults to `Moose::Kokkos::LayoutType::LEFT`).
Both inner and outer arrays can be up to three-dimensional.
However, it is not possible to have inner arrays with different dimensions in a single jagged array.

The accessors of a jagged array, `operator()` (dimensional) or `operator[]` (dimensionless), receive the indices for the outer array.
They return a temporary object that wraps the inner array at the given index, which provides its own accessors that work with the local indices for the inner array.
It will be therefore preferred to store the temporary object locally to avoid the potential overhead of repeated temporary object creation.

Constructing a jagged array is split into two phases: setting up the array structure and populating data.
First, the outer array of a jagged array is created by calling `create()`.
Then, the size of inner arrays should be set one-by-one through `reserve()`.
Once the array structure is set, `finalize()` should be called to construct the dope vectors and allocate data storage.
After finalizing, the jagged array is now ready to be populated.
The following example illustrates the construction sequence of a jagged array:

```cpp
JaggedArray<Real, 2, 2> array;

array.create(n1, n2);

for (unsigned int i = 0; i < n1; ++i)
  for (unsigned int j = 0; j < n2; ++j)
    array.reserve({n1, n2}, {nx[i], ny[j]});

data.finalize();

for (unsigned int i = 0; i < n1; ++i)
  for (unsigned int j = 0; j < n2; ++j)
  {
    auto inner = array(i, j);
    for (unsigned int x = 0; x < nx[i]; ++x)
      for (unsigned int y = 0; y < ny[j]; ++j)
        inner(x, y) = ... // Populating data
  }

data.copyToDevice();
```

!listing framework/include/kokkos/base/KokkosJaggedArray.h
         id=kokkos_jagged_array_source
         caption=The `Moose::Kokkos::JaggedArray` source code.

`Moose::Kokkos::Map` is designed to be used for certain classes of data whose index-based storage using `Moose::Kokkos::Array` is difficult.
It contains a CPU `std::map` and two GPU arrays for storing the keys and values, along with an offset array to store the starting location of each bucket.
It uses the 32-bit FNV-1a hash algorithm on GPU to implement a hash table.
The map can only be populated on CPU, and upon calling `copyToDevice()`, the corresponding hash table is created on GPU.
It also provides `copyToDeviceNested()` for convenience in case the values are of type `Moose::Kokkos::Array`, which calls `copyToDeviceNested()` for each array entry.

The key/value lookup on GPU is index-based.
`find()` returns the index of the value corresponding to the key or `Moose::Kokkos::Map::invalid_id` if the key does not exist.
If the index is valid, you can get the value by using the index with `value()`.
If you know in advance that a key always exists, you can simply call `operator[]` without going through those steps.
However, if `operator[]` is called for a key that does not exist, the calculation will crash.

!listing framework/include/kokkos/base/KokkosMap.h
         id=kokkos_map_source
         caption=The `Moose::Kokkos::Map` source code.

!alert note
If you need to use keys of a custom data type, a specialization of the template function `Moose::Kokkos::fnv1aHash` should be provided for the custom type.

!alert note
`Moose::Kokkos::Map` has not been optimized for performance.
Beware of the performance impact from using it.

!alert note
All the creation and copy APIs of `Moose::Kokkos::Array`, `Moose::Kokkos::JaggedArray`, and `Moose::Kokkos::Map` can only be called by CPU.

### Separate Execution Space id=kokkos_execution_space

Due to different architectures, a GPU function needs to be marked as `KOKKOS_FUNCTION`; if a function is not marked as `KOKKOS_FUNCTION`, it is not callable on GPU.
`KOKKOS_FUNCTION` instructs the compiler to generate both CPU and GPU versions of a function; namely, the function can also be called on CPU.
If the function needs to behave differently on CPU and GPU, you can use `KOKKOS_IF_ON_HOST` and `KOKKOS_IF_ON_DEVICE` to dictate the behaviors separately for CPU and GPU in the same function.
For example:

```cpp
KOKKOS_FUNCTION void * MyClass::getPointer() const
{
  KOKKOS_IF_ON_HOST(return _cpu_pointer;)
  KOKKOS_IF_ON_DEVICE(return _gpu_pointer;)
}
```

It can also be leveraged to throw an error on the CPU side if the function is intended to be GPU only.

Currently, it is required that +all GPU functions should be inlineable+; namely, the callee should be defined in the same source file with the caller or defined in a header file that can be seen by the caller.
The relocatable device code (RDC) feature that allows dynamic linking of GPU functions is currently disabled due to several limitations imposed by PETSc and Kokkos.
In fact, inlining functions has a non-negligible performance advantage on GPU and is encouraged when possible.
However, it can result in excessive number of functions being defined in header files, which reduces code reusability, modularity, and encapsulation and increases dependencies and compilation time.
Therefore, enabling RDC in Kokkos-MOOSE is one of the items with high priority.

### Dynamic Allocation id=kokkos_dynamic_allocation

Dynamic allocation inside a GPU function is prohibitively expensive because of the massive parallelism of GPU.
It is a common practice for CPU programming to dynamically allocate small chunks of memory on-the-fly depending on the needs, but doing so on GPU results in hundreds of thousands of threads performing dynamic allocation simultaneously which are all serialized due to the sequential nature of heap memory allocation.
A way to circumvent this is to preallocate a large GPU array on CPU for all threads and assign a chunk of the array to each thread, but it is very cumbersome and often impossible.
Also, such approach can result in allocating an unnecessarily large memory, as the total number of threads can outweigh the number of physically active threads at a time.
Another way is to define a fixed-sized local array in a GPU function.
+For local arrays whose sizes are small and predictable, this approach gives the best performance and is encouraged.+
However, it will be difficult to set the size in advance if the size can vary significantly, and the size will likely have to be set excessively large.

Therefore, we provide a memory pool algorithm for this purpose that can assign non-overlapping memory chunks to threads and reuse freed chunks in parallel.
A memory pool can be allocated through `MooseApp::allocateKokkosMemoryPool()`.
It requires two arguments: the total memory pool size in bytes and the number of ways.
The number of ways indicates how many parallel pools will be created.
The total pool size is equally divided by each pool; for instance, if the total pool size is 1 GB and there are 1,000 pools, the size of each pool will be 1 MB.
Having more parallel pools reduces the chance of conflicts between threads in each pool and thereby reduces the performance penalty, but at the same time it increases the chance of pool overflow when the requested chunk sizes of threads are not properly balanced.
A performance study indicated that at least thousands of parallel pools are required to minimize performance hits.

The memory pool can be accessed on GPU by calling `kokkosMemoryPool()`, which is available through inheriting the `Moose::Kokkos::MemoryPoolHolder` interface class.
You can obtain a temporary chunk object by calling `allocate()` on the memory pool.
It requires a template argument that specifies the data type and two function arguments: an arbitrary index and the size of chunk in the number of elements.
The index is used to determine the parallel pool index by performing a simple modulo operation on it.
Then, you can obtain the pointer to the assigned memory by calling `get()` on the chunk object.
The allocated chunk is freed when the temporary chunk object is destroyed.
Therefore, the temporary chunk object should be alive while the memory is under use.
The following example illustrates obtaining a memory chunk for 16 `int` values using the thread index:

```cpp
auto chunk = kokkosMemoryPool().allocate<int>(tid, 16);
int * memory = chunk.get();
```

!alert note
For performance, try to minimize requesting chunks and reuse the same chunk as much as possible.

### Functor and Copy Constructor id=kokkos_functor

Every parallel Kokkos object is a +*functor*+ which is a class with one or more overloaded `operator()`s, and they define the parallel work.
Kokkos launches a parallel calculation by dispatching the parallel work of a functor to GPU, and a +single+ copy of the functor is created and copied to GPU during the dispatch.
Namely, the +copy constructor+ of the functor is invoked at every dispatch.
You can leverage this and define a copy constructor of their object to perform synchronizations between CPU and GPU data that need to be done at every dispatch, if there is any.
Also, because there is only a single instance of the functor on GPU, the member variables of the functor are shared by all threads.
Therefore, member variables should not be used to store thread-private data.
In addition, the functor is dispatched as a `const` object, which also copies all the member variables as `const`.
This requires the member functions of the member variables and the functor itself to be `const` functions if they are to be called on GPU.

Since the dispatching invokes the copy constructor of the functor, `Moose::Kokkos::Array` is designed to be shallow-copied by default to avoid unnecessary memory copies during the functor dispatch.
Namely, its copy constructor only performs a shallow copy and does not invoke the copy constructors of its entries.
If it is required to explicitly invoke the copy constructor of each entry for a certain data type used in an array, you should define a specialization of the `Moose::Kokkos::ArrayDeepCopy` type trait template as follows:

```cpp
namespace Moose::Kokkos
{
template <>
struct ArrayDeepCopy<SomeType>
{
  static constexpr bool value = true;
};
}
```

!alert note
The type trait alters the behavior of `Moose::Kokkos::Map` as well which is also shallow-copied by default, because it uses `Moose::Kokkos::Array` internally for storing the values.

### Value Binding id=kokkos_value_binding

Typically, variables retrieved through MOOSE APIs are bound locally in MOOSE objects as references to ensure they always see up-to-date variables.
However, this reference binding occurs on CPU during object construction, and the reference binds a CPU variable.
As the result, the reference is not accessible on GPU due to the separate memory space, and thus binding variables for GPU should not be done using references.
Instead, they should be copied to concrete instances (values) so that their values are copied to the GPU clone during the functor dispatch, which can be described as +*value binding*+.

With the value binding, however, maintaining the synchronization between the original variables and their local bindings is difficult.
Therefore, we provide a template class `Moose::Kokkos::ReferenceWrapper` that can hold the reference of a variable on CPU and provide access to the up-to-date value of the variable on GPU.
It holds the reference of the CPU variable and an instance of the variable at the same time and leverages the copy constructor as the hook to synchronize the two, given that it is invoked at every functor dispatch.
Namely, the copy constructor copies the reference to the instance, which guarantees that the instance has the value of the variable at the moment of functor dispatch.
The wrapper automatically returns the reference on CPU and the instance on GPU, depending on where it is being accessed.

For arithmetic values, there exists `Moose::Kokkos::Scalar` which is a derived class from `Moose::Kokkos::ReferenceWrapper` and provides arithmetic operators that can directly operate on the stored values for non-const types.
For const types, those operators will be undefined.
For example, a postprocessor value which is always provided as read-only can be held by `Moose::Kokkos::Scalar<const PostprocessorValue>` or its alias `Moose::Kokkos::PostprocessrValue`.

!listing framework/include/kokkos/base/KokkosReferenceWrapper.h
         id=kokkos_reference_wrapper_source
         caption=The `Moose::Kokkos::ReferenceWrapper` source code.

!alert note
The data type should be copy-constructable.

### Static Polymorphism with Curiously Recurring Template Pattern (CRTP) id=kokkos_crtp

The primary challenge in porting MOOSE to GPU lies in its heavy reliance on dynamic polymorphism using virtual functions.
Polymorphism is the centerpiece of the +*template method pattern*+, which is a behavioral design pattern in object-oriented programming that establishes the skeleton of an algorithm in a base class while permitting derived classes to override certain steps without altering the algorithm’s overall structure, and it is the key design pattern of MOOSE.

However, virtual functions are not well-supported by GPU backends.
Virtual functions are implemented through virtual function tables (vtable), which are the tables of function pointers implicitly generated by the compiler inside an object to be looked up at runtime.
A vtable is created whenever an object has virtual functions and is populated by the constructor of the object.
Because of this, using virtual functions on GPU requires the object to be instantiated on GPU so that the vtable can be populated with GPU function pointers.
It is cumbersome to implement within the current MOOSE system, because an object instantiated on GPU is not accessible from CPU.
Furthermore, not every GPU backend supports virtual functions.

Aside from portability, using virtual functions on GPU should be avoided if possible for performance, especially when the virtual functions are called in critical paths.
The vtable lookup itself incurs overheads, and using function pointers prevents inlining.
GPU compilers heavily rely on the inlining to generate an optimized code, and being unable to inline functions will likely lead to a performance hit.

Therefore, any polymorphism on GPU should be implemented statically, which can be achieved by the CRTP.
The CRTP is a programming idiom that involves a class template inheriting from a template instantiation of itself, which is a technique used to achieve static (compile-time) polymorphism.
The following pseudo-codes demonstrate a typical template method pattern implemented with the dynamic polymorphism and its equivalent implementation with the static polymorphism using the CRTP:

- Dynamic polymorphism

```cpp
class Base
{
public:
  void compute()
  {
    implementation();
  }
  virtual void implementation() { ... }
};

class Derived : public Base
{
public:
  virtual void implementation() override { ... }
};
```

- Static polymorphism using the CRTP

```cpp
template <typename Derived>
class Base
{
public:
  void compute()
  {
    static_cast<Derived *>(this)->implementation();
  }
  void implementation() { ... }
};

class Derived : public Base<Derived>
{
public:
  void implementation() { ... } // Hides implementation() of Base
};
```

In the pseudo-codes, `compute()` is the +*template method*+ that provides the overall structure of the algorithm, and `implementation()` is the +*hook method*+ that derived classes override to implement specific steps of the algorithm.
Calling the derived class method is resolved at runtime in the dynamic polymorphism, whereas in the CRTP, the derived class type is known at compile time through the template argument, which makes it possible to explicitly cast `this` pointer to the derived class type to call the derived class method.
Overriding a virtual function of the base class is mimicked in the CRTP by +*hiding*+ the function of the base class by redefining it in the derived class.
If the function is not hidden by the derived class, the base class function will be simply called which is available through inheritance.
The behavior of a pure virtual function can also be reproduced by not defining the default function in the base class, so that if you do not define it in the derived class, the compiler will detect it at the call to the derived class function in the base class.
One difference is that in the dynamic polymorphism, the base class can be instantiated by itself unless the function is defined as a pure virtual function, while the self-instantiation is inherently impossible in the CRTP.

The CRTP, however, has several caveats that require extra cautions:

- Inability of Checking Erroneous Function Hiding

The validity of hiding a function that has a default definition in the base class cannot be checked during compile time.
While it will be able to detect incorrect argument type or count to some extent, mistakes such as misspelled function name will result in silently leaving the function unused.
Unlike in dynamic polymorphism where inconsistent overriding can be detected during compile time by explicitly adding the `override` keyword, it cannot be prevented by the compiler.

- Inability of Function Privatization

In the CRTP, the base class calls the derived class method by casting `this` pointer to the derived class type and explicitly calling the method.
As the result, the method being called should be public, which is different from the common practice of encapsulation.

- Complexity of Multiple Inheritance

When there are multiple levels of inheritance, all the intermediate classes should be template classes as well.
Because the class type of the final derived class should be seen by the base class through the template argument, the intermediate classes should relay the class type to the base class.
If you accidentally derive a class from a non-template class, the base class will not be able to see the derived class.
In this case, you are encouraged to prevent unintended inheritance by explicitly adding the `final` keyword to the last level derived class.

The Kokkos-MOOSE base classes are carefully designed to avoid the CRTP by leveraging a registry design pattern where external dispatchers are registered together with the objects.
Namely, the base classes themselves are not template classes, which alleviates the burden of users in dealing with templates.
However, any polymorphic pattern implemented on GPU in the derived class level will likely require the CRTP.

#### Alternative Way to Implement Static Polymorphism id=kokkos_shim

While the CRTP is a generic design pattern for implementing static polymorphism, the use of class templates can complicate class designs.
In Kokkos-MOOSE objects, there is an alternative way to implement static polymorphism by defining shims instead of hook methods.
Analogously to the original MOOSE objects, Kokkos-MOOSE objects also provide the hook methods for the user to implement their own algorithms (e.g., `computeQpResidual()` in [Kokkos Kernels](syntax/KokkosKernels/index.md)).
These hook methods, if implemented in the base class, do not have the information about the derived class type.
Therefore, the user should rely on the CRTP to know the derived class type at compile time if they want to implement a polymorphic pattern in the hook methods.

To alleviate the burden of the CRTP implementation, Kokkos-MOOSE provides additional shims around the hook methods for advanced users.
These shims simply invoke the hook methods and relay the arguments, but they receive an additional argument which is the object itself in its actual type.
Namely, the user can call the desired derived class methods using the actual objects available in the shims.
In case of the [Kokkos Kernels](syntax/KokkosKernels/index.md) as an example, `computeQpResidual()` is called by the shim `computeQpResidualShim()`.
`computeQpResidual()` has the following signature with arbitrary user codes in it:

```cpp
KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                       const unsigned int qp,
                                       AssemblyDatum & datum) const;
{
  // User codes
}
```

And `computeQpResidualShim()` is defined as the following by default, which is a single line function that simply calls `computeQpResidual()`:

```cpp
template <typename Derived>
KOKKOS_FUNCTION Real computeQpResidualShim(const Derived & kernel,
                                           const unsigned int i,
                                           const unsigned int qp,
                                           AssemblyDatum & datum) const
{
  return kernel.computeQpResidual(i, qp, datum);
}
```

If the user defines `computeQpResidualShim()` in their base class with the given signature instead of `computeQpResidual()`, they can customize it to call any method of `kernel` whose type is known in compile time.

### Separate Compilation

Last but not the least, all Kokkos codes should only be included in the source files with a special `*.K` extension.
The Kokkos-MOOSE build system separates the compilation of the main MOOSE and Kokkos codes, avoiding the need to compile the entire MOOSE package with a GPU compiler and making the compilation process modular.
All the Kokkos source files across the framework, modules, and apps are selectively compiled by the GPU compiler and linked into shared libraries by the GPU linker, while the rest of source files are compiled by the standard CPU compiler.
The Kokkos shared libraries are then linked to the final executable by the CPU linker.
The procedure is schematically shown in the following figure.
When the Kokkos capabilities are disabled, the Kokkos source files are simply ignored, making it portable to non-GPU systems.

!media kokkos_separate_compile.png id=fig:kokkos_separate_compile caption=Schematics of the separate compilation of Kokkos source files.

In case Kokkos-related codes (not Kokkos codes) should be added to non-Kokkos source files, the build system conditionally defines a global preprocessor `MOOSE_KOKKOS_ENABLED` when the Kokkos capabilities are enabled.
It can be used to protect the Kokkos-related codes in non-Kokkos source files.
Also, the build system defines another preprocessor `MOOSE_KOKKOS_SCOPE` that is only defined for the Kokkos source files.
It is useful for protecting Kokkos codes in header files that can be included in both Kokkos and non-Kokkos source files.
However, it is important to note that `MOOSE_KOKKOS_SCOPE` should not be used to protect member variables or virtual member functions in a class, as it can make CPU and GPU compiler see inconsistent class definitions having different sizes and cause memory misalignment.
It is primarly used to protect inlined GPU functions with the `KOKKOS_FUNCTION` specifier in header files or non-virtual CPU functions that are not allowed to be called in non-Kokkos source files.

## Kokkos-MOOSE Objects

The following objects are currently available in Kokkos-MOOSE:

- [Kernels](syntax/KokkosKernels/index.md)
- [NodalKernels](syntax/KokkosNodalKernels/index.md)
- [BCs](syntax/KokkosBCs/index.md)
- [Materials](syntax/KokkosMaterials/index.md)
- [AuxKernels](syntax/KokkosAuxKernels/index.md)
- [Functions](syntax/KokkosFunctions/index.md)
- [UserObjects](syntax/KokkosUserObjects/index.md)
- [Postprocessors](syntax/KokkosUserObjects/index.md#reducers)
- [VectorPostprocessors](syntax/KokkosUserObjects/index.md#reducers)
- [Reporters](syntax/KokkosUserObjects/index.md#reducers)

!if-end!

!else
!include kokkos/kokkos_warning.md
