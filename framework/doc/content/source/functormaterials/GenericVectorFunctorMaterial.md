# GenericVectorFunctorMaterial

!syntax description /FunctorMaterials/GenericVectorFunctorMaterial

The functor system allows for using different functor types, functions, variables and
material properties for example, for each component X, Y and Z of the vector functor
material property.

This can be used to quickly create simple constant anisotropic material properties,
for testing, for initial survey of a problem or simply because the material
properties do not vary much over the domain explored by the simulation.

The non-functor equivalents of this material are [GenericConstantVectorMaterial.md] for
constant values and `GenericFunctionVectorMaterial.md` for functions.

By default this class caches function evaluations
and clears the cache at the beginning of every time step. Cache clearing behavior can be
controlled by setting the `execute_on` parameter.

!alert note
All AD-types of the components must match. Variables are automatically considered as AD functors, even
auxiliary variables. The AD version of this material is `ADGenericVectorFunctorMaterial`. Its inputs are a
vector of AD functors and it creates AD vector functor material properties.

## Example Input File Syntax

In this example, we create a `GenericVectorFunctorMaterial` to generate an
anisotropic vector diffusivity and then compute the integral of the diffusive
flux through a specified boundary on the mesh.

!listing test/tests/postprocessors/side_diffusive_flux_integral/side_diffusive_flux_integral.i block=Materials/mat_props_vector

In this example, we create a `GenericVectorFunctorMaterial` for two
anisotropic friction factors in a porous media flow simulation.  Note the syntax
for declaring two material properties and their values in the same material.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc-friction.i block=Materials/darcy

!syntax parameters /FunctorMaterials/GenericVectorFunctorMaterial

!syntax inputs /FunctorMaterials/GenericVectorFunctorMaterial

!syntax children /FunctorMaterials/GenericVectorFunctorMaterial
