# Supporting both AD and non-AD variables through templating

There are a large number of classes that were written before the [!ac](AD) system was
available that cannot leverage the AD system, and cannot therefore be used with the
[finite volume (FV)](/fv_design.md) system which is AD-only.

To avoid duplicating classes that a user may wish to use with the FV system, classes
can be generalized to allow use with both AD and non-AD variables through templating.

These classes are templated on a bool `is_ad`, which is `true` for AD variables, and
`false` for non-AD variables. Several templated methods for coupling variables and
declaring/getting material properties of any type are available, such as

```
coupledGenericValue<is_ad>(var_name)
declareGenericMaterialProperty<T, is_ad>(mat_prop)
getGenericMaterialProperty<T, is_ad>(mat_prop)
```

The following examples in the framework demonstrate how to template a class to use
both AD and non-AD variables and material properties.

## Materials

Consider the [GenericConstantMaterial.md](GenericConstantMaterial.md) material. The header file
defines a templated class `GenericConstantMaterialTempl<is_ad>` with a material
property of `GenericMaterialProperty<Real, is_ad` which evaluates to the correct
type depending on the value of `is_ad`.

!listing framework/include/materials/GenericConstantMaterial.h

Note the `typedef`'s at the end of the header: when `GenericConstantMaterial` is used
in an input file, this class is instantiated with `is_ad = false`, while when
`ADGenericConstantMaterial` is used, `is_ad = true`.

The corresponding source file with templated methods is

!listing framework/src/materials/GenericConstantMaterial.C

Note that both `GenericConstantMaterial` and `ADGenericConstantMaterial` are registered to
the app, and the material property is declared using the templated `declareGenericMaterialProperty<T, is_ad>(mat_prop)`.

## Kernels

Other classes can be templated in a similar fashion. Consider a [Kernel](BodyForce.md) example. In this case,
the class is derived from the `GenericKernel` base class, to ensure that the `computeQpResidual()` method
returns the correct type for both AD and non-AD variables (note that it is of type `GenericReal<is_ad>`).
Otherwise, the basic concept is the same as before.

!listing framework/include/kernels/BodyForce.h

The corresponding source file is

!listing framework/src/kernels/BodyForce.C

One important observation in this case is that this class (`BodyForcdeTempl`) derives
from a templated base class (`GenericKernel<is_ad`). This is slightly more complicated
than the material example above. Members of the base class are not available in this
derived class without including them with the `using` declaration in the header file
(`usingGenericKernelMembers`), which is defined in the `GenericKernel` header

!listing framework/include/kernels/GenericKernel.h

In addition, templated methods used in a class derived from a templated base class (like in
example above) must be prefixed with `this->template` to avoid compiler ambiguity, for example,
the use of `getParam<T>` in the `BodyForce` kernel above

!listing
_scale(this->template getParam<Real>("value")),
