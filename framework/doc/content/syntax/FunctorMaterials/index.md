# FunctorMaterials

Functor material properties are properties that are evaluated on-the-fly.
Along with numerous classes of MOOSE objects, they are [Functors](syntax/Functors/index.md). This allows
for the creation of objects with considerable inter-operability between systems.

## Using functor materials

Much like regular materials declare regular material properties, functor materials declare functor material properties.
Following a producer/consumer model, `FunctorMaterials` produce functor material properties, which kernels, boundary conditions, other
functor materials, etc, may consume.
Both regular and functor material properties support a wide array of data types (scalar, vector, tensors,
[automatic differentiation](automatic_differentiation/index.md) versions of all of these). Functor materials, because they are evaluated on-the-fly
and call other functors on-the-fly as well, do not require dependency ordering.

All functor materials support caching in some capacity. This can avoid expensive material property computations, but is
disabled by default due to the potential memory cost. The reader is referred to the [Functors caching documentation](syntax/Functors/index.md#caching)

!alert warning
Functor materials are generally **NOT** compatible with regular materials, as regular material properties **CANNOT** be used
in lieu of functor material properties. Functor material properties can only be used as regular material properties using a
[MaterialFunctorConverter.md] to process them.

Functor materials can be created within the `[FunctorMaterials]` block, and currently the `[Materials]` block as well,
but that syntax is considered for deprecation.

!alert note
If a Functor is reported as missing by the simulation, and it was supposed to be created by a `FunctorMaterial`,
you may use the [!param](/Debug/SetupDebugAction/show_functors) to get more information about what functors were
created and requested.

## Developing with functor materials

Functor material properties are properties that are evaluated
on-the-fly. E.g. they can be viewed as functions of the current location in
space (and time). Functor material properties provide several overloads of the
`operator()` method for different "geometric quantities". One example of a
"geometric quantity" is a `const Elem *`, e.g. for an `FVElementalKernel`, the
value of a functor material property in a cell-averaged sense can be obtained by
the syntax

- `_foo(_current_elem)`

where here `_foo` is a functor material property data member of the kernel. The
functor material property system introduces APIs very similar to the traditional
material property system for declaring and getting properties. To declare a
functor property:

- `declareFunctorProperty<TYPE>`

where `TYPE` can be anything such as `Real, ADReal, RealVectorValue, ADRealVectorValue`
etc. To get a functor material property:

- `getFunctor<TYPE>`

It's worth noting that whereas the traditional regular material property system
has different methods to declare/get non-AD and AD properties, the new functor
system has single APIs for both non-AD and AD property types.

Currently, functor material property evaluations are defined using the API:

```c++
template <typename T>
template <typename PolymorphicLambda>
void FunctorMaterialProperty<T>::
setFunctor(const MooseMesh & mesh,
           const std::set<SubdomainID> & block_ids,
           PolymorphicLambda my_lammy);
```

where the first two arguments are used to setup block restriction and the last argument is a lambda
defining the property evaluation. The lambda must be callable with two arguments, the first
corresponding to space, and the second corresponding to time, and must return the type `T` of the
`FunctorMaterialProperty`. An example of setting a constant functor material property that returns
an `ADReal` looks like:

```c++
    _constant_unity_prop.setFunctor(
        _mesh, blockIDs(), [](const auto &, const auto &) -> ADReal { return 1.; });
```

An example of a functor material property that depends on a nonlinear variable would look like

```c++
    _u_prop.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
      return _u_var(r, t);
    });
```

In the above example, we simply forward the calling arguments along to the variable. Variable
functor implementation is described in [MooseVariableBase.md#functor-vars]. A test functor material
class to setup a dummy Euler problem is shown in

!listing test/src/materials/ADCoupledVelocityMaterial.C

In the following subsections, we describe the various spatial arguments that functor (material
properties) can be evaluated at. Almost no functor material developers should have to concern
themselves with these details as most material property functions should just appear as functions of
space and time, e.g. the same lambda defining the property evaluation should apply across all
spatial and temporal arguments. However, in the case that a functor material developer wishes to
create specific implementations for specific arguments (as illustrated in `IMakeMyOwnFunctorProps`
test class) or simply wishes to know more about the system, we give the details below.

Any call to a functor (material property) looks like the following
`_foo(const SpatialArg & r, const TemporalArg & t)`. Below are the possible type overloads of
`SpatialArg`.

### FaceArg id=spatial-overloads

A typedef defining a "face" evaluation calling argument. This is composed of

- a face information object which defines our location in space
- a limiter which defines how the functor evaluated on either side of the face should be
  interpolated to the face
- a boolean which states whether the face information element is upwind of the face
- a pair of subdomain IDs. These do not always correspond to the face info element subdomain
  ID and face info neighbor subdomain ID. For instance if a flux kernel is operating at a
  subdomain boundary on which the kernel is defined on one side but not the other, the
  passed-in subdomain IDs will both correspond to the subdomain ID that the flux kernel is
  defined on

### ElemQpArg

Argument for requesting functor evaluation at a quadrature point location in an element. Data
in the argument:

- The element containing the quadrature point
- The quadrature point index, e.g. if there are `n` quadrature points, we are requesting the
  evaluation of the ith point
- The quadrature rule that can be used to initialize the functor on the given element

If functor material properties are functions of nonlinear degrees of freedom, evaluation with this
argument will likely result in calls to libMesh `FE::reinit`.

### ElemSideQpArg

Argument for requesting functor evaluation at quadrature point locations on an element side.
Data in the argument:

- The element
- The element side on which the quadrature points are located
- The quadrature point index, e.g. if there are `n` quadrature points, we are requesting the
  evaluation of the ith point
- The quadrature rule that can be used to initialize the functor on the given element and side

If functor material properties are functions of nonlinear degrees of freedom, evaluation with this
argument will likely result in calls to libMesh `FE::reinit`.

### Functor caching

By default, functor material properties are always (re-)evaluated every time they are called with
`operator()`. However, the base class that `FunctorMaterialProperty` inherits from,
`Moose::Functor`, has a
`setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule)` API that allows
control of evaluations. Supported values for the `clearance_schedule` are any combination of
`EXEC_ALWAYS`, `EXEC_TIMESTEP_BEGIN`, `EXEC_LINEAR`, and `EXEC_NONLINEAR`. These will cause cached
evaluations of functor (material properties) to be cleared always (in fact not surprisingly in this
case we never fill the cache), on `timestepSetup`, on `residualSetup`, and on `jacobianSetup`
respectively. If a functor is expected to depend on nonlinear degrees of freedom, then the cache
should be cleared on `EXEC_LINEAR` and `EXEC_NONLINEAR` (the default `EXEC_ALWAYS` would obviously also work) in
order to achieve a perfect Jacobian. Not surprisingly, if a functor evaluation is cached, then
memory usage will increase.

!alert note title=Caching Implementations
Functor caching is only currently implemented for `ElemQpArg` and `ElemSideQpArg` spatial
overloads. This is with the idea that calls to `FE::reinit` can be fairly expensive whereas for the
other spatial argument types, evaluation of the functor (material property) may be relatively
inexpensive compared to the memory expense incurred from caching. We may definitely implement
caching for other overloads, however, if use cases call for it.
