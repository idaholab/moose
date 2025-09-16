# GenericConstantVectorMaterial

!syntax description /Materials/GenericConstantVectorMaterial

This can be used for example to create simple constant anisotropic material properties, for testing,
for initial survey of a problem or simply because the anisotropic material properties do not vary much over the
domain explored by the simulation.

!alert note
If you are working with a `std::vector<double>`-valued material property, e.g. a variable-size
vector, you have to use the [GenericConstantStdVectorMaterial.md].

## Example Input File Syntax

In this example, we create a `GenericConstantVectorMaterial` to generate an
anisotropic vector diffusivity and then compute the integral of the diffusive
flux through a specified boundary on the mesh.

!listing test/tests/postprocessors/side_diffusive_flux_integral/side_diffusive_flux_integral.i block=Materials/mat_props_vector

!syntax parameters /Materials/GenericConstantVectorMaterial

!syntax inputs /Materials/GenericConstantVectorMaterial

!syntax children /Materials/GenericConstantVectorMaterial
