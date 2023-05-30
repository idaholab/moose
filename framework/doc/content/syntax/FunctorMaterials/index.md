# FunctorMaterials

Much like regular materials declare regular material properties, functor materials declare functor material properties.
Functor material properties are properties that are evaluated on-the-fly.
Along with multiple other classes of MOOSE objects, they are [Functors](syntax/Functors/index.md). This allows
for the creation of objects with considerable inter-operability between systems.

## Using functor materials

Following a producer/consumer model, `FunctorMaterials` produce functor material properties, which kernels, boundary conditions, other
functor materials, etc, may consume.
Both regular and functor material properties support a wide array of data types (scalar, vector, tensors,
[automatic differentiation](automatic_differentiation/index.md) versions of all of these). Functor materials, because they are evaluated on-the-fly
and call other functors on-the-fly as well, do not require dependency ordering.

All functor materials support caching in some capacity. This can avoid expensive material property computations, but is
disabled by default due to the potential memory cost. The reader is referred to the [Functors caching documentation](syntax/Functors/index.md#caching).

!alert warning
Functor material properties and regular material properties are generally **NOT** compatible with each other. Functor material properties can only be used as regular material properties using a
[MaterialFunctorConverter.md] to process them.

Functor materials are created within the `[FunctorMaterials]` block.

!alert note
If a Functor is reported as missing by the simulation, and it was supposed to be created by a `FunctorMaterial`,
you may use the `Debug/`[!param](/Debug/SetupDebugAction/show_functors) parameter to get more information about what functors were
created and requested.

## Developing with functor materials

### Evaluating functors

See the [Functor system documentation](syntax/Functors/index.md#using-functors) for information on
how to retrieve a functor material property value in a kernel, boundary condition, ... even another functor
material property.

### Creating functor material properties

The functor material property system introduces APIs slightly different from the traditional
material property system for declaring/adding and getting properties. To add a
functor property:

- `addFunctorProperty<TYPE>`

where `TYPE` can be anything such as `Real, ADReal, RealVectorValue, ADRealVectorValue`
etc. To get a functor material property:

- `getFunctor<TYPE>`

It's worth noting that whereas the traditional regular material property system
has different methods to declare/get non-AD and AD properties, the new functor
system has single APIs for both non-AD and AD property types.

Functor material property evaluations are defined using the API:

```c++
template <typename T, typename PolymorphicLambda>
const Moose::FunctorBase<T> &
FunctorMaterial::addFunctorProperty(const std::string & name,
                                    PolymorphicLambda my_lammy,
                                    const std::set<ExecFlagType> & clearance_schedule = {
                                        EXEC_ALWAYS});
```

where the first argument will be the functor name stored in the problem functor database (all functor names must be unique), the second argument is a lambda
defining the property evaluation, and the third optional argument is a set defining at which execution stages the functor evaluation cache should be cleared. The lambda must be callable with two arguments, the first
corresponding to space, and the second corresponding to time, and must return the type `T` of the
`FunctorMaterialProperty`. An example of adding a constant functor material property that returns
a `Real` looks like:

```c++
    addFunctorProperty<Real>(
        "foo", [](const auto &, const auto &) -> Real { return 1.; });
```

An example of a functor material property that depends on a fluid properties user object and pressure and temperature functors looks like

```c++
  addFunctorProperty<ADReal>(_density_name,
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fluid.rho_from_p_T(_pressure(r, t), _temperature(r, t)); });
```

In the above example, we simply forward the calling arguments along to the captured functors.
`_pressure` and `_temperature` are captured in the lambda function because they are attributes to the local
`FunctorMaterial` class, and `[this]` in the function definition captures all attributes from the class.
Variable functor implementation is described in [MooseVariableBase.md#functor-vars]. A test functor material
class to setup a dummy Euler problem is shown in

!listing test/src/materials/ADCoupledVelocityMaterial.C

### Spatial arguments

See the general [Functor documentation](syntax/Functors/index.md#spatial-overload) for an explanation about the spatial arguments that
may be used for functor material properties.

### Value Caching

See the general [Functor documentation](syntax/Functors/index.md#caching) for an explanation about the
caching capabilities that may be used with functor material properties.
