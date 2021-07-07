# Material System

A system for defining material properties to be used by multiple systems and allow for variable
coupling.

!---

The material system operates by creating a producer/consumer relationship among objects

- `Material` objects +produce+ properties.
- Other MOOSE objects (including materials) +consume+ these properties.

!---

## Producing Properties

1. Each property to be produced must be declared to be available for use, the
   `declareProperty<TYPE>()` method does this and returns a writable reference.
1. Override `computeQpProperties()` to compute all of the declared properties at one quadrature point.
   Within this method, the references obtained from declaring the property are updated.

!---

## Consuming Properties

To consume a material property, call the correct get method in an object and store the
constant reference as a member variable.

`getMaterialProperty<TYPE>()`\\
Use within non-AD objects to retrieve non-AD material properties.

`getADMaterialProperty<TYPE>()`\\
Use within AD objects to retrieve AD material properties.


!---

## Material Property Evaluation

Quantities are recomputed at quadrature points, as needed.

Multiple `Material` objects may define the same "property" for different parts of the subdomain or
boundaries.

!---

## Stateful Material Properties

The values are not stored between timesteps unless "stateful" properties are enabled, which is
accomplished by calling `getMaterialPropertyOld<TYPE>()` or `getMaterialPropertyOlder<TYPE>()`

It can be useful to have "old" values of `Material` properties available in a simulation, such as
in solid mechanics plasticity constitutive models.

Traditionally, this type of value is called a "state variable"; in MOOSE, they are called
"stateful material properties".

Stateful `Material` properties require more memory.

!---

## Default Material Properties

Default values for material properties may be assigned within the `validParams` function.

```cpp
addParam<MaterialPropertyName>("combination_property_name", 12345,
 "The name of the property providing the luggage combination");
```

Only scalar (`Real`) values may have defaults.

When `getMaterialProperty<Real>("combination_property_name")` is called, the default will be returned
if the value has not been computed via a `delcareProperty` call within a `Material` object.

!---

## Material Property Output

Output of `Material` properties is enabled by setting the "outputs" parameter.

The following example creates two additional variables called "mat1" and "mat2" that will show up in
the output file.

!listing output_block.i block=Materials Outputs

!---

## Supported Property Types for Output

`Material` properties can be of arbitrary (C++) type, but not all types can be output.

| Type | AuxKernel | Variable Name(s) |
| :- | :- | :- |
| Real | `MaterialRealAux` | prop |
| RealVectorValue | `MaterialRealVectorValueAux` | prop_1, prop_2, and prop_3 |
| RealTensorValue | `MaterialRealTensorValueAux` | prop_11, prop_12, prop_13, prop_21, etc. |
