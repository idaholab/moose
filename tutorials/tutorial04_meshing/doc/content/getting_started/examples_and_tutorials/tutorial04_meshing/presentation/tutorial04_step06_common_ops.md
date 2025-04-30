# Frequently Used Operations and Corresponding MeshGenerators

!---

## Mesh Trimming Along Lines of Symmetry

!row!
!col! width=66%

- [HexagonMeshTrimmer.md]
- (Cartesian sibling -- [CartesianMeshTrimmer.md])

- Two types of trimming can be performed by [HexagonMeshTrimmer.md]: Peripheral Trimming and Through-the-Center Trimming.

- Peripheral trimming can be performed on six possible lines, each of which is parallel to a side of the hexagon and crosses the center of the pins laid out in that direction
- Peripheral trimming can only be used for assembly meshes
- Through-the-center trimming can be used for both assembly and core meshes

!col-end!

!col! width=33%

!media tutorial04_meshing/base_ex_hmt.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!---

## Mesh Trimming Examples

!row!
!col! width=50%

!media tutorial04_meshing/base_ex_peripheral_trim.png
       style=width:70%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!col! width=50%

!media tutorial04_meshing/base_ex_center_trim.png
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!row!
!col! width=50%

!listing base_mesh_generators/common_geo.i
         block=Mesh/pattern_assm_peri_trim
         link=False

!col-end!

!col! width=50%

!listing base_mesh_generators/common_geo.i
         block=Mesh/core_trim
         link=False

!col-end!

!row-end!

!---

## Assembly Periphery Modification

!row!
!col! width=66%

- [PatternedHexPeripheralModifier.md]
- (Cartesian sibling -- [PatternedCartesianPeripheralModifier.md])

- Modify the peripheral region of an assembly mesh to enforce a given number of nodes uniformly distributed on the external boundary to facilitate the stitching of different assembly meshes.

- The input mesh must be an assembly with a hexagonal boundary (coolant and/or duct region(s) present).

!col-end!

!col! width=33%

!media tutorial04_meshing/base_ex_phpm.png
       style=width:80%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!listing base_mesh_generators/common_geo.i
         block=Mesh/pattern_assm_peri_mod
         link=False

!---

## Extrusion to 3D

!row!
!col! width=66%

- [AdvancedExtruderGenerator.md]
- Extrudes a 1D mesh into 2D, or 2D into 3D
- Variable height / # of layers in each height
- Variable growth factors of axial element sizes within each elevation
- Remap subdomain IDs, boundary IDs and element EEIDs in each elevation and boundaries between neighboring elevations
- Extrusion may be performed along any direction specified by an $(x,y,z)$ vector. Most common is $(0,0,1)$ (+$z$-direction).

!col-end!

!col! width=33%

!media tutorial04_meshing/base_ex_aeg.png
       style=width:80%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!listing base_mesh_generators/common_geo.i
         block=Mesh/core_ext
         link=False
