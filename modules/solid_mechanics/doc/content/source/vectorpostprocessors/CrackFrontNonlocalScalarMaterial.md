# CrackFrontNonlocalScalarMaterial

!syntax description /VectorPostprocessors/CrackFrontNonlocalScalarMaterial

## Description

This object computes the average of a scalar material property in the region of the crack front points defined by  [CrackFrontDefinition.md].  The main use case for this `VectorPostprocessor` is to compute an average fracture toughness or $K_c$ at the crack front for use with the `MeshCut2DFractureUserObject` to grow cracks. This allows for spatially varying $K_c$ values defined by a `Material`.

`CrackFrontNonlocalScalarMaterial` computes an average of the material property over a box-shaped domain at each crack tip point that is centered on the crack tip and extends [!param](/VectorPostprocessors/CrackFrontNonlocalScalarMaterial/box_length) in front of the crack tip.  The [!param](/VectorPostprocessors/CrackFrontNonlocalScalarMaterial/box_height) is the dimension normal to the crack face, and [!param](/VectorPostprocessors/CrackFrontNonlocalScalarMaterial/box_width) is the dimension tangential to the crack face.  [!param](/VectorPostprocessors/CrackFrontNonlocalScalarMaterial/box_width) is not used in 2D problems.

In the following input file example, the mesh consists of a 3D plate with a hole in the middle. The CrackFrontDefinition defines crack points around the center line of the hole, `boundary=1001`. This `CrackFrontNonlocalScalarMaterial` averages a material property named `scalar_kcrit` over each 3D box at each crack front point.

!listing crack_front_nonlocal_materials.i block=UserObjects VectorPostprocessors/CrackFrontNonlocalKcrit

!syntax parameters /VectorPostprocessors/CrackFrontNonlocalScalarMaterial

!syntax inputs /VectorPostprocessors/CrackFrontNonlocalScalarMaterial

!syntax children /VectorPostprocessors/CrackFrontNonlocalScalarMaterial
