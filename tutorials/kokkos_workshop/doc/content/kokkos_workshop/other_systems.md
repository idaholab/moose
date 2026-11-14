# Part 6: Other Systems id=kokkos_workshop_systems

!---

## Integrated Boundary Conditions

Same interface as kernels: `Moose::Kokkos::IntegratedBC` with `computeQpResidual(i, qp, datum)`
etc., or the optimized `Moose::Kokkos::IntegratedBCValue` with `precomputeQpResidual(qp, datum)`
when the residual is $(\psi_i, f)$. Register with `registerKokkosResidualObject()`.

!listing modules/heat_transfer/include/kokkos/bcs/KokkosConvectiveHeatFluxBC.h start=class

`datum.normals(qp)` gives the outward normal on the side.

!---

## Nodal BCs and NodalKernels

There are no test or trial function indices; `qp` is always 0 but is kept for a uniform
interface:

```cpp
template <typename Derived>
KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int qp, AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int jvar, const unsigned int qp,
                                              AssemblyDatum & datum) const;
```

- `_current_node` is replaced by `datum.node()` (contiguous node ID); the coordinate is
  `datum.q_point(qp)`
- Couple nodal values with `kokkosCoupledNodalValue("v")`
- `Moose::Kokkos::NodalBC` and `Moose::Kokkos::NodalKernel` share this interface

!--

## Nodal BC Example: `KokkosMatchedValueBC`

!listing framework/include/kokkos/bcs/KokkosMatchedValueBC.h start=class

For Dirichlet-type conditions, derive from `Moose::Kokkos::DirichletBCBase` and provide only
`computeValue(qp, datum)`; the base class calls it through `static_cast<const Derived *>(this)`.

!---

## AuxKernels

Derive from `Moose::Kokkos::AuxKernel`, register with `registerKokkosAuxKernel()`:

```cpp
template <typename Derived>
KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;
```

- One interface for elemental and nodal variables; query `isNodal()` on host or device.
  `datum.node()` is only valid for nodal variables.
- `uOld()`, `uOlder()`, and all `kokkosCoupled*()` methods are available
- Not supported yet: boundary-restricted elemental AuxKernels, vector and array AuxKernels
- Advanced: redefine `computeElementInternal()` / `computeNodeInternal()` to write a custom loop
  (see `KokkosCopyValueAux`)

!--

## AuxKernel Example: Arrays of Coupled Values

From `KokkosVariableTimeIntegrationAux`, which stores a variable number of coupled values in a
`Moose::Kokkos::Array`:

!listing framework/src/kokkos/auxkernels/KokkosVariableTimeIntegrationAux.K start=_integration_coef.create end=switch include-end=False language=cpp

```cpp
  ... // fill entries on the host, e.g. _coupled_vars[1] = kokkosCoupledValueOld("variable_to_integrate");

  _integration_coef.copyToDevice();
  _coupled_vars.copyToDevice();
```

On the device: `_coupled_vars[i](datum, qp)`.

!---

## Functions

Derive from `Moose::Kokkos::FunctionBase` (+not+ `Moose::Kokkos::Function`), register with
`registerKokkosFunction()`, and define any of these as inlined public methods:

```cpp
KOKKOS_FUNCTION Real value(Real t, Real3 p) const;
KOKKOS_FUNCTION Real3 vectorValue(Real t, Real3 p) const;
KOKKOS_FUNCTION Real3 gradient(Real t, Real3 p) const;
KOKKOS_FUNCTION Real timeDerivative(Real t, Real3 p) const;
KOKKOS_FUNCTION Real timeIntegral(Real t1, Real t2, Real3 p) const;
... // curl(), div(), integral(), average()
```

!listing framework/include/kokkos/functions/KokkosConstantFunction.h start=class

!---

## Using Functions: Know the Concrete Type

On GPUs a function must be retrieved +by its concrete type+ so the call can be inlined:

```cpp
// Header
Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _func;

// Constructor
_func(getKokkosFunction<KokkosParsedFunction>("function"))

// Device
const Real f = _func->value(_t, datum.q_point(qp));
```

- The type-erased `Moose::Kokkos::Function` returned by the non-template `getKokkosFunction()`
  relies on virtual dispatch and is +rejected on GPU builds+ until relocatable device code is
  available; it does work with `--with-kokkos=cpu`
- For prototyping, `KokkosParsedFunction`, `KokkosParsedMaterial`, and `KokkosParsedAux` evaluate
  expressions from the input file on the device

!---

## UserObjects

Supported bases (register all with `registerKokkosUserObject()`):

- `Moose::Kokkos::ElementUserObject`, `SideUserObject`, `NodalUserObject`, and the
  Postprocessor / VectorPostprocessor / Reporter variants built on them
- `Moose::Kokkos::GeneralUserObject` (no parallel loop; registered with `registerMooseObject()`)

The parallel hook replaces `execute()`:

```cpp
template <typename Derived>
KOKKOS_FUNCTION void execute(Datum & datum) const;
```

- `initialize()`, `finalize()`, `getValue()` are ordinary host methods; there is no `threadJoin()`
- Dependencies between Kokkos and host user objects are +not+ resolved automatically; use
  `execution_order_group` if order matters. Within a group, Kokkos user objects run first.

!---

## Reducers: Postprocessors Done Right

Accumulating into a member (`_sum += ...`) is a data race. Define the +reducer+ hooks instead,
and Kokkos performs a parallel reduction into a buffer of `Real`s:

```cpp
template <typename Derived>
KOKKOS_FUNCTION void reduce(Datum & datum, Real * result) const; // accumulate one element
template <typename Derived>
KOKKOS_FUNCTION void join(Real * result, const Real * source) const; // combine partial results
template <typename Derived>
KOKKOS_FUNCTION void init(Real * result) const; // identity of the reduction
```

1. `allocateReductionBuffer(n)` in the constructor or `initialize()`
1. Kokkos calls `init`, `reduce`, `join` on the device
1. Results land in `_reduction_buffer` (host); communicate in `finalize()` with `gatherSum()`
   and friends, return from `getValue()`

!--

## Reducer Example: `KokkosIntegralPostprocessor`

!listing framework/include/kokkos/postprocessors/KokkosIntegralPostprocessor.h start=template <typename Base>
         end=typedef

`computeQpIntegral()` is supplied by the derived class, e.g. `KokkosElementL2Norm` returns
`u * u` and takes the square root in `getValue()`.

!--

## Dense vs. Sparse Reductions

- Reducers suit +small, dense+ buffers: every `reduce()` call touches most entries
- If each call only touches a few entries of a large buffer (histograms, per-ID integrals), each
  partial reduction still initializes and joins the whole buffer, and large buffers can exceed
  the scratch memory (`requested too much L0 scratch memory`)
- Use an ordinary parallel `execute()` with atomic updates (`::Kokkos::atomic_add`) instead
- `KokkosExtraIDIntegralVectorPostprocessor` implements both strategies

!---

# Exercise 6: An AuxKernel and a Reducer Postprocessor id=kokkos_workshop_ex6

!---

## Exercise 6: Tasks

1. Write `KokkosGradientMagnitudeAux`, an elemental AuxKernel that stores $|\nabla v|$ of a
   coupled variable `v`. Error out in the constructor if it is used on a nodal variable.
1. Write `KokkosVolumeFractionAbove`, a postprocessor on `KokkosElementVariablePostprocessor`
   returning the fraction of the domain volume where `variable > threshold`. Use the reducer
   hooks.
1. Add both to the transient problem of Exercise 5 (`MONOMIAL`/`CONSTANT` auxiliary variable for the
   gradient magnitude) and check the fraction against a refined mesh
1. Bonus: why would accumulating the volumes into two members in an ordinary `execute()` hook
   give wrong answers?

!--

## Exercise 6: Solution, AuxKernel

!listing tutorials/kokkos_workshop/kokkos_training/include/auxkernels/KokkosGradientMagnitudeAux.h start=class link=False

!listing tutorials/kokkos_workshop/kokkos_training/src/auxkernels/KokkosGradientMagnitudeAux.K start=KokkosGradientMagnitudeAux::KokkosGradientMagnitudeAux( language=cpp link=False

!--

## Exercise 6: Solution, Postprocessor Header

!listing tutorials/kokkos_workshop/kokkos_training/include/postprocessors/KokkosVolumeFractionAbove.h start=class link=False

!--

## Exercise 6: Solution, Postprocessor Source

!listing tutorials/kokkos_workshop/kokkos_training/src/postprocessors/KokkosVolumeFractionAbove.K start=#include language=cpp link=False

Input: `tests/ex6/ex6.i`; the `ex6/nodal_error` test checks the error for nodal variables.

!--

## Exercise 6: Discussion

- `execute()` runs on one shared, `const` copy of the object: members cannot be written from
  device threads, and making them `mutable` turns the accumulation into a data race
- The reducer gives every partial reduction its own buffer, initialized by `init()` and combined
  by `join()`, so no two threads write the same memory
- MPI ranks still need `gatherSum()` in `finalize()`: the Kokkos reduction only spans one device
