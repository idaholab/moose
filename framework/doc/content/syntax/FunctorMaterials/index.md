# FunctorMaterials

Functor material properties are properties that are evaluated on-the-fly.
Along with multiple other classes of MOOSE objects, they are [Functors](syntax/Functors/index.md). This allows
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
but that syntax will be deprecated.

!alert note
If a Functor is reported as missing by the simulation, and it was supposed to be created by a `FunctorMaterial`,
you may use the [!param](/Debug/SetupDebugAction/show_functors) to get more information about what functors were
created and requested.

## Developing with functor materials

Functor material properties are properties that are evaluated
on-the-fly. E.g. they can be viewed as functions of the current location in
space (and time). Functor material properties provide several overloads of the
`operator()` method for different "geometric quantities". One example of a
"geometric quantity" is based around an element, e.g. for an `FVElementalKernel`, the
value of a functor material property in a cell-averaged sense can be obtained by
the syntax

- `_foo(makeElemArg(_current_elem), determineState())`

where here `_foo` is a functor data member of the kernel, `makeElemArg` is a helper routine for creating a
functor element-based spatial argument, and `determineState()` is a helper routine for determining the correct
time state to evaluate at, e.g. the current time for an implicit kernel and the old time for an explicit kernel.
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

Currently, functor material property evaluations are defined using the API:

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
