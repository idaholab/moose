# Part 2: The Programming Model id=kokkos_workshop_model

!---

## Five Rules

Almost every porting problem comes from one of these:

1. +Separate memory+: device code cannot see host memory
1. +Separate execution+: device code can only call `KOKKOS_FUNCTION`s that are visible (inlined)
1. +Objects are functors+: one `const` copy of your object is shipped to the device per launch
1. +No virtual dispatch on the device+: polymorphism is static (templates)
1. +No dynamic allocation on the device+

The rest of this part explains each rule and the tools Kokkos-MOOSE provides for it.

!---

## Rule 1: Separate Memory Spaces

On most GPUs the host and the device have different memory. Anything allocated by the host is
unreachable from device code:

- `std::vector`, `std::map`, `std::set`, `std::string`, smart pointers
- References and raw pointers to host data
- libMesh objects: `Elem *`, `Node *`, `MeshBase`, `FEBase`, `NumericVector`
- MOOSE host objects: `MooseVariable`, `MaterialProperty<T>`, `Function`

Kokkos-MOOSE provides device-capable replacements:

| Host | Device-capable |
| :- | :- |
| `std::vector<T>`, multi-dimensional arrays | `Moose::Kokkos::Array<T, dim>` |
| `std::vector<std::vector<T>>` | `Moose::Kokkos::JaggedArray<T, inner, outer>` |
| `std::map<K, V>` | `Moose::Kokkos::Map<K, V>` (slow, use sparingly) |
| `const VariableValue &` | `Moose::Kokkos::VariableValue` |
| `const MaterialProperty<T> &` | `Moose::Kokkos::MaterialProperty<T, dim>` |
| `const Real &` (controllable, postprocessor) | `Moose::Kokkos::Scalar<const Real>`, `Moose::Kokkos::PostprocessorValue` |

!---

## `Moose::Kokkos::Array`

```cpp
Moose::Kokkos::Array<Real> coef;   // 1D
coef.create(n);                    // allocate on host AND device (createHost/createDevice for one side)
for (const auto i : make_range(n))
  coef[i] = ...;                   // fill on the host
coef.copyToDevice();               // explicit synchronization, nothing is automatic

Moose::Kokkos::Array<Real> w = getParam<std::vector<Real>>("weights"); // allocates and copies both

Moose::Kokkos::Array<Real, 3> table; // or Array3D<Real>
table.create(nx, ny, nz);
table(i, j, k) = 1;                  // multi-dimensional index, first index varies fastest
```

- Accessors return host data on the host and device data on the device
- Copying an `Array` is +shallow+ (cheap); this matters because your object is copied at every
  launch
- Arrays of arrays: populate the inner arrays, then call `copyToDeviceNested()` on the outermost
- Device memory is +uninitialized+ after allocation
- Creation and copy methods are host-only

!---

## Rule 2: Separate Execution Spaces

- A function callable on the device must be marked `KOKKOS_FUNCTION`; the compiler then generates
  host and device versions
- Device functions must be +inlineable+: define them in headers (or in the same `.K` file as the
  caller). Relocatable device code is not enabled, so device functions cannot be linked across
  translation units.
- Use the Kokkos versions of math functions on the device: `::Kokkos::sin`, `::Kokkos::exp`,
  `::Kokkos::abs`, `::Kokkos::max`, `::Kokkos::sqrt`
- No exceptions, no `mooseError()`, no `std::cout` on the device: use `::Kokkos::abort("...")`
  and `::Kokkos::printf(...)`

Different behavior on each side:

```cpp
KOKKOS_FUNCTION void * MyClass::getPointer() const
{
  KOKKOS_IF_ON_HOST(return _cpu_pointer;)
  KOKKOS_IF_ON_DEVICE(return _gpu_pointer;)
}
```

!---

## Rule 3: Objects Are Functors

A Kokkos parallel loop executes a +functor+, an object with an `operator()`. In Kokkos-MOOSE your
kernel, material, or user object +is+ the functor.

- At each launch, +one+ copy of the object is made with its +copy constructor+ and shipped to the
  device
- All device threads share that single copy, and it is `const`
- Therefore:

  - Hook methods and every member function they call must be `const`
  - Members cannot hold per-thread scratch data (they are shared and read-only)
  - Per-element and per-quadrature-point data comes in through the `datum` argument instead
  - The copy constructor is a natural place to refresh device data before every launch

!---

## Value Binding

In MOOSE you bind data by reference so that you always see the current value:

```cpp
const VariableValue & _u;          // host reference, invisible on the device
const Real & _scale;               // controllable parameter
const PostprocessorValue & _pp;
```

On the device, references to host memory are useless. Kokkos-MOOSE objects bind +by value+, and
wrappers keep the copy current:

```cpp
const Moose::Kokkos::VariableValue _u;          // light handle, copied to the device
const Moose::Kokkos::Scalar<const Real> _scale; // re-read from the host reference at every launch
const Moose::Kokkos::PostprocessorValue _pp;    // = Scalar<const PostprocessorValue>
```

`Moose::Kokkos::ReferenceWrapper<T>` (base of `Scalar`) holds the host reference and a copy, and
refreshes the copy in its copy constructor, which runs at every launch.

!--

## Value Binding Example: `KokkosBodyForce`

!listing framework/include/kokkos/kernels/KokkosBodyForce.h start=class

!listing framework/src/kokkos/kernels/KokkosBodyForce.K start=KokkosBodyForce::KokkosBodyForce( language=cpp

!---

## Rule 4: No Virtual Dispatch on the Device

- Objects are constructed on the host, so their vtables point to +host+ functions
- Virtual calls cannot be inlined, and GPU compilers depend on inlining

Hook methods are therefore +public, non-virtual function templates+ on the final object type,
and they +hide+ (not override) the base class versions:

```cpp
class MyKernel : public Moose::Kokkos::Kernel
{
public:
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
};
```

[Part 4](#kokkos_workshop_poly) shows how this works and how to build your own polymorphic
objects on it.

!---

## Rule 5: No Dynamic Allocation on the Device

Thousands of threads calling `new` or `malloc` at once serialize on the device heap.

- Small, bounded scratch space: use a +fixed-size local array+ (fastest)

  ```cpp
  Real local[MAX_SIZE]; // MAX_SIZE is a compile-time constant
  ```

- Large or unpredictable sizes: preallocate on the host as an `Array`, or use the Kokkos-MOOSE
  memory pool (`MooseApp::allocateKokkosMemoryPool()` and `kokkosMemoryPool().allocate<T>(idx,
  n)` through `Moose::Kokkos::MemoryPoolHolder`)
- Material property types must be +trivial+ (no constructors, no virtual functions, no heap
  members); use multi-dimensional properties instead of `std::vector` properties

!---

# Exercise 2: Spot the Bug id=kokkos_workshop_ex2

!---

## Exercise 2: What Is Wrong With Each Snippet?

Work in pairs. Each snippet compiles (or almost does) but is wrong for Kokkos-MOOSE.

!style! fontsize=70%

!row!
!col! width=50%

```cpp
// (A)
class A : public Moose::Kokkos::Kernel
{
  ...
  const Moose::Kokkos::VariableValue & _v;
};
```

```cpp
// (B)
class B : public Moose::Kokkos::Kernel
{
  ...
  std::vector<Real> _weights;
};
// used as _weights[qp] in computeQpResidual
```

```cpp
// (C)
template <typename Derived>
KOKKOS_FUNCTION Real
C::computeQpResidual(const unsigned int i,
                     const unsigned int qp,
                     AssemblyDatum & datum) const
{
  _sum += _u(datum, qp); // mutable Real _sum;
  return _sum * _test(datum, i, qp);
}
```

!col-end!

!col! width=50%

```cpp
// (D)
class D : public Moose::Kokkos::Kernel
{
public:
  virtual Real computeQpResidual(const unsigned int i,
                                 const unsigned int qp,
                                 AssemblyDatum & datum) const;
};
```

```cpp
// (E) Conductivity.h
KOKKOS_FUNCTION Real conductivity(Real T);
// Conductivity.K
KOKKOS_FUNCTION Real conductivity(Real T) { ... }
// E.h: E::computeQpResidual() calls conductivity()
```

```cpp
// (F)
template <typename Derived>
KOKKOS_FUNCTION Real
F::computeQpJacobain(const unsigned int i,
                     const unsigned int j,
                     const unsigned int qp,
                     AssemblyDatum & datum) const
{
  return std::exp(_u(datum, qp)) * _phi(datum, j, qp)
         * _test(datum, i, qp);
}
```

!col-end!
!row-end!

!style-end!

!--

## Exercise 2: Answers (1/2)

- +(A) Reference binding+. The reference points to a host object. Store the handle by value:
  `const Moose::Kokkos::VariableValue _v;`
- +(B) Host container on the device+. `std::vector` memory lives on the host. Use
  `Moose::Kokkos::Array<Real> _weights;`, fill it on the host, call `copyToDevice()` (or assign
  from a `std::vector`, which copies to both sides).
- +(C) Shared member used as scratch+. All threads share one copy of the object: this is a data
  race (and `mutable` only hides the `const` error). Use a local variable; if you really need a
  running total, it belongs in a reducer (Part 6).

!--

## Exercise 2: Answers (2/2)

- +(D) Virtual hook+. Device code never calls it through the vtable. Hooks are non-virtual
  `template <typename Derived> KOKKOS_FUNCTION` methods.
- +(E) Device function defined in another translation unit+. Without relocatable device code
  the device compiler cannot link `conductivity()` into `E.K`. Define it `inline` in
  `Conductivity.h`.
- +(F) Two bugs+:

  - Misspelled `computeQpJacobain`: compiles, but the Jacobian is never computed. Newton
    convergence degrades, and only a Jacobian test reveals why.
  - `std::exp` is not guaranteed to be callable on the device: use `::Kokkos::exp`.

!--

## Exercise 2: All Fixes in One Compiled Kernel

`KokkosPolynomialReaction` adds $(\psi_i, e^u - p(v))$ for a polynomial $p$ with input
coefficients. Its comments mark where each fix applies (`tests/ex2` in the solutions
application):

!style! fontsize=80%

!listing tutorials/kokkos_workshop/kokkos_training/include/kernels/KokkosPolynomialReaction.h start=class link=False

!listing tutorials/kokkos_workshop/kokkos_training/include/utils/KokkosPolynomial.h start=/** link=False

!listing tutorials/kokkos_workshop/kokkos_training/src/kernels/KokkosPolynomialReaction.K start=KokkosPolynomialReaction::KokkosPolynomialReaction( language=cpp link=False

!style-end!
