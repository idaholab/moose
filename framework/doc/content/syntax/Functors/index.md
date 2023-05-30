# Functor system

Functors are an abstraction, existing as a base class, that is available to several systems in MOOSE:

- [Variables](syntax/Variables/index.md)
- [Auxiliary variables](syntax/AuxVariables/index.md)
- [Functor material properties](syntax/FunctorMaterials/index.md)
- [Functions](syntax/Functions/index.md)

All functors can be called using the same interfaces. This enables considerable code re-use.
For example, instead of having a kernel for each type of coupled forcing term, like
[CoupledForce.md], [BodyForce.md], [MatCoupledForce.md], [ADMatCoupledForce.md], we could just have a single object
`FunctorForce` and have the force term be a functor.

`Functions` provide a good analogy to `Functors`. Both `Functions` and `Functors` are evaluated on-the-fly at a location in space and time,
but for `Functors`, the space arguments can be an element, a point in an element,
a face of an element, and so on. The time arguments represent the state of the functor : current, previous value (whether in time or iteration),
or value before that previous one.

!alert note
If a Functor is reported as missing by the simulation, you may use the `Debug/`[!param](/Debug/SetupDebugAction/show_functors)
parameter to get more information about which functors were created and requested.

## Developing with functors

Functors are stored on the `SubProblem`, a derived class of [Problems](syntax/Problem/index.md) that is
used for solving nonlinear systems. As such, classes do not need to have memory ownership of functors,
they may simply store a reference or a pointer.

In the header of a class using a `Functor` `_functor` you will have:

```
const Moose::Functor<T> & _functor
```

to store a reference to `_functor`. `T` is the return type of the functor. For a variable, it should be `ADReal`.
For a vector variable, it would be `ADRealVectorValue`. If the object using the functor is not leveraging AD,
it may be `Real` or `RealVectorValue`.

!alert note
With regards to [automatic differentiation](automatic_differentiation/index.md), `Functors` are automatically
converted between `AD` and `non-AD` types when retrieved. When you retrieve a `Functor`, you must only think about whether
you need `AD` in the consuming object. When you create a `Functor` (for example, a
[functor material property](syntax/FunctorMaterials/index.md)) it's best practice to always use the `AD`
return type so as to never discard some derivatives.

In the constructor of the same class, you will have:
```
CLASSNAME::CLASSNAME(const InputParameters & parameters) :
  ...
  _functor(getFunctor<T>("<functor_parameter_name>")),
  ...
```

where `CLASSNAME` is the name of the class, `T` is still the required return type of the functor, and `<functor_parameter_name>` is
the name of the parameter used for providing the functor in the input.

### Evaluating functors id=using-functors

Functors are evaluated on-the-fly. E.g. they can be viewed as functions of the current location in
space (and time). Functors provide several overloads of the
`operator()` method for different "geometric quantities". One example of a
"geometric quantity" is based around an element, e.g. for an `FVElementalKernel`, the
value of a functor material property in a cell-averaged sense can be obtained by
the syntax

- `_foo(makeElemArg(_current_elem), determineState())`

where here `_foo` is a functor data member of the kernel, `makeElemArg` is a helper routine for creating a
functor element-based spatial argument, and `determineState()` is a helper routine for determining the correct
time state to evaluate at, e.g. the current time for an implicit kernel and the old time for an explicit kernel.

### Spatial arguments to functors id=spatial-overloads

In the following subsections, we describe the various spatial arguments that functors can be evaluated at.
Almost no functor developers should have to concern
themselves with these details as most functor definitions should just appear as functions of
space and time, e.g. the same lambda defining the property evaluation should apply across all
spatial and temporal arguments. However, in the case that a functor developer wishes to
create specific implementations for specific arguments (as illustrated in `IMakeMyOwnFunctorProps`
test class) or simply wishes to know more about the system, we give the details below.

Any call to a functor looks like the following
`_foo(const SpatialArg & r, const TemporalArg & t)`. Below are the possible type overloads of
`SpatialArg`.

#### FaceArg

A `struct` defining a "face" evaluation calling argument. This is composed of

- a face information object which defines our location in space
- a limiter which defines how the functor evaluated on either side of the face should be
  interpolated to the face
- a boolean which states whether the face information element is upwind of the face
- a boolean to indicate whether to correct for element skewness
- a pointer to an element indicating if there is sidedness to the face, and if so, the side of the face.
  If null, the evaluation should use information from both sides of the face, if the functor is defined on both sides.
  If not null, the evaluation should ignore information from the other element.

#### ElemArg

Argument for requesting functor evaluation at an element. This is often used to evaluate
constant monomial or finite volume variables.
Data in the argument:

- The element of interest
- Whether to correct for element skewness

#### ElemQpArg

Argument for requesting functor evaluation at a quadrature point location in an element. Data
in the argument:

- The element containing the quadrature point
- The quadrature point index, e.g. if there are `n` quadrature points, we are requesting the
  evaluation of the ith point
- The quadrature rule that can be used to initialize the functor on the given element

If functors are functions of nonlinear degrees of freedom, evaluation with this
argument will likely result in calls to libMesh `FE::reinit`.

#### ElemSideQpArg

Argument for requesting functor evaluation at quadrature point locations on an element side.
Data in the argument:

- The element
- The element side on which the quadrature points are located
- The quadrature point index, e.g. if there are `n` quadrature points, we are requesting the
  evaluation of the ith point
- The quadrature rule that can be used to initialize the functor on the given element and side

If functors are functions of nonlinear degrees of freedom, evaluation with this
argument will likely result in calls to libMesh `FE::reinit`.

#### ElemPointArg

Argument for requesting functor evaluation at a point located inside an element.
Data in the argument:

- The element containing the point
- The point to evaluate the functor at
- Whether to correct for element skewness

#### Nodes

There is currently no nodal argument to functors.
Please contact a MOOSE developer if you need this.

### Functor caching

By default, functors are always (re-)evaluated every time they are called with
`operator()`. However, the base class `Moose::Functor` has a
`setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule)` API that allows
control of evaluations (in addition to the `clearance_schedule` argument to `addFunctorProperty` introduced above).
Supported values for the `clearance_schedule` are any combination of
`EXEC_ALWAYS`, `EXEC_TIMESTEP_BEGIN`, `EXEC_LINEAR`, and `EXEC_NONLINEAR`. These will cause cached
evaluations of functors to be cleared always (in fact not surprisingly in this
case we never fill the cache), on `timestepSetup`, on `residualSetup`, and on `jacobianSetup`
respectively. If a functor is expected to depend on nonlinear degrees of freedom, then the cache
should be cleared on `EXEC_LINEAR` and `EXEC_NONLINEAR` (the default `EXEC_ALWAYS` would obviously also work) in
order to achieve a perfect Jacobian. Not surprisingly, if a functor evaluation is cached, then
memory usage will increase.

!alert note title=Caching Implementations
Functor caching is only currently implemented for `ElemQpArg` and `ElemSideQpArg` spatial
overloads. This is with the idea that calls to `FE::reinit` can be fairly expensive whereas for the
other spatial argument types, evaluation of the functors may be relatively
inexpensive compared to the memory expense incurred from caching. We may definitely implement
caching for other overloads, however, if use cases call for it.
