# Materials System

The material system is the primary mechanism for defining spatially varying properties. The system
allows properties to be defined in a single object (a `Material`) and shared among the many other
systems such as the [Kernel](syntax/Kernels/index.md) or [BoundaryCondition](syntax/BCs/index.md)
systems. Material objects are designed to directly couple to solution variables as well as other
materials and therefore allow for capturing the true nonlinear behavior of the equations.

The material system relies on a producer/consumer relationship: `Material` objects +produce+
properties and other objects (including materials) +consume+ these properties.

The properties are produced on demand, thus the computed values are always up to date. For example, a
property that relies on a solution variable (e.g., thermal conductivity as function of temperature)
will be computed with the current temperature during the solve iterations, so the properties are
tightly coupled.

The material system supports the use of automatic differentiation for property calculations, as such
there are two approaches for producing and consuming properties: with and without automatic
differentiation. The following sections detail the producing and consuming properties using the
two approaches. To further understand automatic differentiation, please refer to the
[automatic_differentiation/index.md] page for more information.

The proceeding sections briefly describe the different aspects of a `Material` object for
producing and computing the properties as well as how other objects consume the properties. For an
example of how a `Material` object is created and used please refer to
[ex08_materials.md optional=True].

## Producing/Computing Properties

Properties must be produced by a `Material` object by declaring the property with one of two methods:

1. `declareProperty<TYPE>("property_name")` declares a property with a name "property_name" to be
   computed by the `Material` object.
1. `declareADProperty<TYPE>` declares a property with a name "property_name" to be
   computed by the `Material` object that will include automatic differentiation.

The `TYPE` is any valid C++ type such an `int` or `Real` or `std::vector<Real>`. The properties must
then be computed within the `computeQpProperties` method defined within the object.

The property name is an arbitrary name of the property, this name should be set such that it
corresponds to the value be computed (e.g., "diffusivity"). The name provided here is the same name
that will be used for consuming the property. More information on names is provided in
[#property-names] section below.

For example, consider a simulation that requires a diffusivity term. In the `Material` object
header a property is declared (in the C++ since) as follows.

!listing ex08_materials/include/materials/ExampleMaterial.h line=_diffusivity

All properties will either be a `MaterialProperty<TYPE>` or `ADMaterialProperty<TYPE>`
and must be a non-const reference. Again, the `TYPE` can be any C++ type. In this example, a scalar
`Real` number is being used.

In the source file the reference is initialized in the initialization list using the aforementioned
declare functions as follows. This declares the property (in the material property sense) to be
computed.

!listing ex08_materials/src/materials/ExampleMaterial.C line=_diffusivity(declare

The final step for producing a property is to compute the value. The computation occurs within a
`Material` object `computeQpProperties` method. As the method name suggests, the purpose of the
method is to compute the values of properties at a quadrature point. This method is a virtual method
that must be overridden. To do this, in the header the virtual method is declared (again in the C++
sense).

!listing ex08_materials/include/materials/ExampleMaterial.h line=computeQpProperties

In the source file the method is defined. For the current example this definition computes the
"diffusivity" as well another term, refer to [ex08_materials.md optional=True].

!listing ex08_materials/src/materials/ExampleMaterial.C start=ExampleMaterial::computeQpProperties

The purpose of the content of this method is to assign values for the properties at a quadrature
point. Recall that "_diffusivity" is a reference to a `MaterialProperty` type. The `MaterialProperty`
type is a container that stores the values of a property for each quadrature point. Therefore, this
container must be indexed by `_qp` to compute the value for a specific quadrature point.

## Consuming Properties

Objects that require material properties consume them using one of two functions

1. `getMaterialProperty<TYPE>("property_name")` retrieves a property with a name "property_name" to be
   consumed by the object.
1. `getADMaterialProperty<TYPE>("property_name")` retrieves a property with a name "property_name" to be
   consumed by the object that will include automatic differentiation.

For an object to consume a property the same basic procedure is followed. First in the consuming
objects header file a `MaterialProperty` with the correct type (e.g., `Real` for the diffusivity
example) is declared (in the C++ sense) as follows. Notice, that the member variable is a +const+
reference. The const is important. Consuming objects cannot modify a property, it only uses the
property so it is marked to be constant.

!listing ex08_materials/include/kernels/ExampleDiffusion.h line=_diffusivity

In the source file the reference is initialized in the initialization list using the aforementioned
get methods. This method initializes the `_diffusivity` member variable to reference the
desired value of the property as computed by the material object.

!listing ex08_materials/src/kernels/ExampleDiffusion.C line=_diffusivity(get

The name used in the get method, "diffusivity", in this case is not arbitrary. This name corresponds
with the name used to declare the property in the material object.

!alert note title=The declare/get calls must correspond
If a material property is declared for automatic differentiation (AD) using `declareADProperty`
then it +must+ be consumed with the `getADMaterialProperty`. The same is true for non-automatic
differentiation; properties declared with `declareProperty` +must+ be consumed with the
`getMaterialProperty` method.

### Optional Properties

Objects can weakly couple to material properties that may or may not exist.

1. `getOptionalMaterialProperty<TYPE>("property_name")` retrieves an optional property with a name "property_name" to be consumed by the object.
1. `getOptionalADMaterialProperty<TYPE>("property_name")` retrieves an optional property with a name "property_name" to be consumed by the object that will include automatic differentiation.

This API returns a reference to an optional material property
(`OptionalMaterialProperty` or  `OptionalADMaterialProperty`). If the requested
property is not provided by any material this reference will evaluate to
`false`. It is the consuming object's responsibility to check for this before
accessing the material property data. Note that the state of the returned
reference is only finalized _after_ all materials have been constructed, so a
validity check must _not_ be made in the constructor of a material class but
either at time of first use in `computeQpProperties` or in `initialSetup`.

## Property Names id=property-names

When creating a Material object and declaring the properties that shall be computed, it is often
desirable to allow for the property name to be changed via the input file. This may be accomplished
by adding an input parameter for assigning the name. For example, considering the example above
the following code snippet adds an input parameter, "diffusivity_name", that allows the input
file to set the name of the diffusivity property, but by default the name remains "diffusivity".

```c++
params.addParam<MaterialPropertyName>("diffusivity_name", "diffusivity",
                                      "The name of the diffusivity material property.");
```

In the material object, the declare function is simply changed to use the parameter name rather
than string by itself. By default a property will be declared with the name "diffusivity".

!listing ex08_materials/src/materials/ExampleMaterial.C line=_diffusivity(declare replace=["diffusivity", "diffusivity_name"]

However, if the user wants to alter this name to something else, such as "not_diffusivity" then
the input parameter "diffusivity_name" is simply added to the input file block for the
material.

```text
[Materials]
  [example]
    type = ExampleMaterial
    diffusivity_name = not_diffusivity
  []
[]
```

On the consumer side, the get method will now be required to use the name "not_diffusivity" to
retrieve the property. Consuming objects can also use the same procedure to allow for custom
property names by adding a parameter and using the parameter name in the get method in the same
fashion.


## Default Material Properties

The `MaterialPropertyName` input parameter also provides the ability to set default values for scalar
(`Real`) properties. In the above example, the input file can use number or
parsed function (see [MooseParsedFunction.md]) to define a the property value. For example, the input
snippet above could set a constant value.

```text
[Materials]
  [example]
    type = ExampleMaterial
    diffusivity_name = 12345
  []
[]
```

## Stateful Material Properties

In general properties are computed on demand and not stored. However, in some cases values of
material properties from a previous timestep may be required. To access properties two
methods exist:

- `getMaterialPropertyOld<TYPE>` returns a reference to the property from the previous timestep.
- `getMaterialPropertyOlder<TYPE>` returns a reference to the property from two timesteps before the
  current.

This is often referred to as a "state" variable, in MOOSE we refer to them as "stateful material
properties." As stated, material properties are usually computed on demand.


!alert warning title=Stateful properties will increase memory use
When a stateful property is requested through one of the above methods this is no longer the
case. When it is computed the value is also stored for every quadrature point on every element. As
such, stateful properties can become memory intensive, especially if the property being stored is a
vector or tensor value.

## Material Property Output

Output of `Material` properties is enabled by setting the "outputs" parameter. The following example
creates two additional variables called "mat1" and "mat2" that will show up in the output file.

!listing output_block.i block=Materials Outputs

`Material` properties can be of arbitrary (C++) type, but not all types can be output. The following
table lists the types of properties that are available for automatic output.

| Type | AuxKernel | Variable Name(s) |
| :- | :- | :- |
| Real | `MaterialRealAux` | prop |
| RealVectorValue | `MaterialRealVectorValueAux` | prop_1, prop_2, and prop_3 |
| RealTensorValue | `MaterialRealTensorValueAux` | prop_11, prop_12, prop_13, prop_21, etc. |

## Material sorting

Materials are sorted such that one material may consume a property produced by
another material and know that the consumed property will be up-to-date,
e.g. the producer material will execute before the consumer material. If a
cyclic dependency is detected between two materials, then MOOSE will produce an
error.

## Functor Material Properties id=functor-props

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

## Advanced Topics

### Evaluation of Material Properties on Element Faces

MOOSE creates three copies of a *non-boundary restricted* material for evaluations on quadrature points of elements, element faces on both the current element side and the neighboring element side.
The name of the element interior material is the material name from the input file, while the name of the element face material is the material name appended with `_face` and the name of the neighbor face material is the material name appended with `_neighbor`.
The element material can be identified in a material with its member variable `_bnd=false`.
The other two copies have `_bnd=true`.
The element face material and neighbor face material differentiate with each other by the value of another member variable `_neighbor`.
If a material declares multiple material properties and some of them are not needed on element faces, users can switch off their declaration and evaluation based on member variable `_bnd`.

### Interface Material Objects

MOOSE allows a material to be defined on an internal boundary of a mesh with a specific material type `InterfaceMaterial`.
Material properties declared in interface materials are available on both sides of the boundary.
Interface materials allows users to evaluate the properties on element faces based on quantities on both sides of the element face.
Interface materials are often used along with [InterfaceKernel](syntax/InterfaceKernels/index.md).

### Discrete Material Objects

A "[Discrete](http://www.dictionary.com/browse/discrete)" `Material` is an object that may be
detached from MOOSE and computed explicitly from other objects. An object inheriting from
[MaterialPropertyInterface](http://www.mooseframework.org/docs/doxygen/moose/classMaterialPropertyInterface.html)
may explicitly call the compute methods of a `Material` object via the `getMaterial` method.

The following should be considered when computing `Material` properties explicitly.

- It is possible to disable the automatic computation of a `Material` object by MOOSE by setting
  the `compute=false` parameter.
- When `compute=false` is set the compute method (`computeQpProperties`) is +not+ called by MOOSE,
  instead it must be called explicitly in your application using the `computeProperties` method
  that accepts a quadrature point index.
- When `compute=false` an additional method should be defined, `resetQpProperties`, which sets the
  properties to a safe value (e.g., 0) for later calls to the compute method. Not doing this can
  lead to erroneous material properties values.

The original intent for this functionality was to enable to ability for material properties to be
computed via iteration by another object, as in the following example. First, consider define a
material (`RecomputeMaterial`) that computes the value of a function and its derivative.

!equation
f(p) = p^2v

and

!equation
f'(p) = 2pv,

where v is known value and not a function of p. The following is the compute portion of this object.

!listing RecomputeMaterial.C start=MOOSEDOCS_START include-start=False

Second, define another material (`NewtonMaterial`) that computes the value of $p: f(p)=0$ using
Newton iterations. This material declares a material property (`_p`) which is what is solved for by
iterating on the material properties containing `f` and `f'` from `RecomputeMaterial`. The
`_discrete` member is a reference to a `Material` object retrieved with `getMaterial`.

!listing NewtonMaterial.C start=MOOSEDOCS_START include-start=Falseg

To create and use a "Discrete" `Material` use the following to guide the process.

1. Create a `Material` object by, in typical MOOSE fashion, inheriting from the `Material` object in
   your own application.
1. In your input file, set `compute=false` for this new object.
1. From within another object (e.g., another Material) that inherits from `MaterialPropertyInterface`
   call the `getMaterial` method. Note, this method returns a reference to a `Material` object, be
   sure to include `&` when calling or declaring the variable.
1. When needed, call the `computeProperties` method of the `Material` being sure to provide the
   current quadrature point index to the method (`_qp` in most cases).


!syntax list /Materials objects=True actions=False subsystems=False

!syntax list /Materials objects=False actions=False subsystems=True

!syntax list /Materials objects=False actions=True subsystems=False
