# Kokkos Materials System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [Materials System](syntax/Materials/index.md) to understand the MOOSE material system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

!alert note
Kokkos-MOOSE materials do not support automatic differention yet.

A Kokkos-MOOSE material can be created by subclassing `Moose::Kokkos::Material`.
Note that it should now be registered with `registerKokkosMaterial()` instead of `registerMooseObject()`.
The hook method for material property computation, which used to be:

```cpp
virtual void computeQpProperties() override;
```

in the original MOOSE materials, is now defined as a +*inlined public*+ method with the following signature:

```cpp
KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;
```

Other than the hook method definition, the Kokkos-MOOSE materials have several notable differences with the original MOOSE materials in the handling of material properties.
The material property producer and consumer methods now have the signatures of

- `declareKokkosProperty<type, dimension>(name, dims)`, and
- `getKokkosMaterialProperty<type, dimension, state>(name)`,

respectively.
Unlike in the original MOOSE materials where the `type` can be any valid C++ type, Kokkos-MOOSE materials require it to be a +*trivial type*+.
Namely, it should not contain any virtual functions, dynamically allocated member variables, user-defined constructors, and other member variables of non-trivial types.
While the code will not prevent using non-trivial types, it is your responsibility to understand the implications of using them and to make them behave properly.
For instance, using a class with a user-defined constructor will have to be manually initialized as it is not called automatically upon allocation.
Same applies to a class with in-class member initialization, which is equivalent to having a user-defined constructor.
Using a class with dynamic allocations will [incur a significant performance hit](syntax/Kokkos/index.md#kokkos_dynamic_allocation) and will break when it is used for stateful material properties.

Instead, the material properties in Kokkos-MOOSE can be multi-dimensional to partially support the needs for dynamically-sized material properties.
The dimension is provided as the second template argument `dimension`, which has the default value of 0 (scalar) and can be up to 4.
The size of each dimension is provied as a vector as the function argument `dims`.
It requires a material property to have the same dimension and size at every quadrature point.

The material properties are stored as an object of type `Moose::Kokkos::MaterialProperty<type, dimension>`.
Note that any material property object [should be stored as a concrete instance](syntax/Kokkos/index.md#kokkos_value_binding).
The producer and consumer methods also return the material property objects as copies.
The only containers that are allowed to hold material properties dynamically are `Moose::Kokkos::Array` and `Moose::Kokkos::Map`.

The material property values of a quadrature point is accessed via `operator()` of `Moose::Kokkos::MaterialProperty` with `datum` and `qp` as arguments.
It creates and returns a temporary object of type `Moose::Kokkos::MaterialPropertyValue<type, dimension>` which is a thin object that retrives the property values of the current quadrature point.
Consider storing this temporary object locally to avoid object creation overhead every time it is called.
For scalar properties, the temporary object can directly be cast into the property data type and overloads `operator=` so that a value can be directly assigned to it.
For multi-dimensional properties, it provides `operator()` with dimensional indices like `Moose::Kokkos::Array`.
The following examples illustrate the usage of scalar and multi-dimensional material properties, respectively:

- Scalar (`Moose::Kokkos::MaterialProperty<unsigned int>`)

```cpp
// Store the value
unsigned int value1 = _property1(datum, qp);
// Store the temporary object
auto prop1 = _property1(datum, qp);
auto prop2 = _property2(datum, qp);

// Compute material property (all are equivalent)
_property2(datum, qp) = value1 + 1;
_property2(datum, qp) = prop1 + 1;
prop2 = value1 + 1;
prop2 = prop1 + 1;
```

- Multi-dimensional (`Moose::Kokkos::MaterialProperty<unsigned int, 3>`)

```cpp
// Store the temporary object
auto prop = _property(datum, qp);

// Compute material property
for (unsigned int i = 0; i < n1; ++i)
  for (unsigned int j = 0; j < n2; ++j)
    for (unsigned int k = 0; k < n3; ++k)
      prop(i, j, k) = i + j + k;
```

See the following source codes of `KokkosGenericConstantMaterial` for an example of a material:

!listing framework/include/kokkos/materials/KokkosGenericConstantMaterial.h id=kokkos-constant-mat-header
         caption=The `KokkosGenericConstantMaterial` header file.

!listing framework/src/kokkos/materials/KokkosGenericConstantMaterial.K id=kokkos-constant-mat-source language=cpp
         caption=The `KokkosGenericConstantMaterial` source file.

## Stateful Material Properties

Stateful material properties can be obtained by `getKokkosMaterialPropertyOld<type, dimension>(name)` or `getKokkosMaterialPropertyOlder<type, dimension>(name)`, or by specifying the state number (0 for current, 1 for old, and 2 for older) explicitly as the third template argument `state` of `getKokkosMaterialProperty<type, dimension, state>(name)` which defaults to 0.
Stateful material properties can be optionally initialized by defining the following +*inlined public*+ hook method:

```cpp
KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int qp, Datum & datum) const
```

See the following source codes of `KokkosStatefulTest` for an example of stateful material properties:

!listing test/include/kokkos/materials/KokkosStatefulTest.h id=kokkos-stateful-mat-header
         caption=The `KokkosStatefulTest` header file.

!listing test/src/kokkos/materials/KokkosStatefulTest.K id=kokkos-stateful-mat-source language=cpp
         caption=The `KokkosStatefulTest` source file.

## On-Demand Properties

In Kokkos-MOOSE, all material property data are stored for each quadrature point, regardless of whether the material properties are stateful or not.
This is due to the massive parallelization of GPU that prevents storing material properties only for a single element or face.
If you are unsure whether a material property will be actually requested by another object, you can declare the property as an on-demand property with `declareKokkosOnDemandProperty<type, dimension>(name, dims)`.
The storage of an on-demand property is only allocated when there is any object consuming the property, aiding in reducing the memory usage and computational cost for unused material properties.

## Optional Properties

There is no special method and object for weakly coupled material properties in Kokkos-MOOSE.
Instead, a material property object will simply evaluate to `false` when it is uninitialized or when it holds an on-demand material property not requested by any object.
You can query the existence of a material property with `hasKokkosMaterialProperty<type, dimension>(name)` and optionally initialize the material property object to reproduce the same behavior with the optional material properties in the original MOOSE.

See the following source codes of `KokkosVarCouplingMaterial` for an example of optional material properties:

!listing test/include/kokkos/materials/KokkosVarCouplingMaterial.h id=kokkos-var-coupling-mat-header
         caption=The `KokkosVarCouplingMaterial` header file.

!listing test/src/kokkos/materials/KokkosVarCouplingMaterial.K id=kokkos-var-coupling-mat-source language=cpp
         caption=The `KokkosVarCouplingMaterial` source file.

## Material Property Output

Material property output is not supported by Kokkos-MOOSE yet.

## Advanced Topics

### Evaluation of Material Properties on Element Faces

Analogously to the original MOOSE, Kokkos-MOOSE also creates three copies of materials for element and face material properties, which are distinguished by the combination of boolean flags `_bnd` and `_neighbor`.
For Kokkos-MOOSE, it is crucial to optimize your material by switching off the declaration and evaluation of material properties that are not used on faces, because of the full storage of material properties.
You can save a considerable amount of memory as well as computing time by switching off unused material properties.
You can also leverage on-demand material properties to achieve the same effect.

### Unsupported Material Types

The following material types are not supported by Kokkos-MOOSE yet:

- Functor materials
- Interface materials
- Discrete materials

!syntax list /Materials objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md
