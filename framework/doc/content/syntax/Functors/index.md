# Functor system

Functors are an abstraction, existing as a base class, that is available to numerous systems in MOOSE:

- [Variables](syntax/Variables/index.md)
- [Auxiliary variables](syntax/AuxVariables/index.md)
- [Functor material properties](syntax/FunctorMaterials/index.md)
- [Functions](syntax/Functions/index.md)

All functors can be called using the same interfaces. This enables considerable code re-use.
For example, instead of having a kernel for each type of coupled forcing term, like
[CoupledForce.md], [BodyForce.md], [MatCoupledForce.md], [ADMatCoupledForce.md], we could just have a single object
`FunctorForce.md` and have the force term be a functor.

`Functions` provide a good analogy to `Functors`. `Functions` are evaluated on-the-fly at a location in space and time.
`Functors` are evaluated on-the-fly with a space and a time argument. The space arguments can be an element, a point in an element,
a face of an element, and so on. The time arguments represent the state of the functor : current, previous value (whether in time or iteration),
or value before that previous one.

## Spatial arguments to functors

In the following subsections, we describe the various spatial arguments that functor can be evaluated at.
Almost no functor developers should have to concern
themselves with these details as most functor definitions should just appear as functions of
space and time, e.g. the same lambda defining the property evaluation should apply across all
spatial and temporal arguments. However, in the case that a functor developer wishes to
create specific implementations for specific arguments (as illustrated in `IMakeMyOwnFunctorProps`
test class) or simply wishes to know more about the system, we give the details below.

Any call to a functor looks like the following
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

If a functor is a function of nonlinear degrees of freedom, evaluation with this
argument will likely result in calls to libMesh `FE::reinit`.

### ElemSideQpArg

Argument for requesting functor evaluation at quadrature point locations on an element side.
Data in the argument:

- The element
- The element side on which the quadrature points are located
- The quadrature point index, e.g. if there are `n` quadrature points, we are requesting the
  evaluation of the ith point
- The quadrature rule that can be used to initialize the functor on the given element and side

If a functor is a function of nonlinear degrees of freedom, evaluation with this
argument will likely result in calls to libMesh `FE::reinit`.

## Functor caching id=caching

By default, functors are always (re-)evaluated every time they are called with
`operator()`. However, the base class of functors, `Moose::Functor`, has a
`setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule)` API that allows
control of evaluations. Supported values for the `clearance_schedule` are any combination of
`EXEC_ALWAYS`, `EXEC_TIMESTEP_BEGIN`, `EXEC_LINEAR`, and `EXEC_NONLINEAR`. These will cause cached
evaluations of functor to be cleared always (in fact not surprisingly in this
case we never fill the cache), on `timestepSetup`, on `residualSetup`, and on `jacobianSetup`
respectively. If a functor is expected to depend on nonlinear degrees of freedom, then the cache
should be cleared on `EXEC_LINEAR` and `EXEC_NONLINEAR` (the default `EXEC_ALWAYS` would obviously also work) in
order to achieve a perfect Jacobian. Not surprisingly, if a functor evaluation is cached, then
memory usage will increase.

!alert note title=Caching Implementations
Functor caching is only currently implemented for `ElemQpArg` and `ElemSideQpArg` spatial
overloads. This is with the idea that calls to `FE::reinit` can be fairly expensive whereas for the
other spatial argument types, evaluation of the functor may be relatively
inexpensive compared to the memory expense incurred from caching. We may definitely implement
caching for other overloads, however, if use cases call for it.
