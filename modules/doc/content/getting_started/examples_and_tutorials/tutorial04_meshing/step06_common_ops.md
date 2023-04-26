# Frequently Used Operations and Corresponding MeshGenerators

## Mesh Trimming Along Lines of Symmetry

Reactor cores are often designed with lines of symmetry through the core center, which in turn entails a symmetric solution. To save computational resources, the mesh can be cut along lines of symmetry and reflective boundary conditions can be applied to the solution at the cut interface. Through-the-center mesh trimming is required to cut the mesh along these symmetry lines.

An alternative type of symmetry occurs in certain "unit cells" which contain half-pins on the edge of the unit cell. When repeated (mirrored) on the lines of symmetry, these half-pins become full pins and part of a larger core structure. To create these unit cells, an assembly with multiple pins is created (with intact pins), and then peripheral trimming is applied to the assembly along any or all of six possible lines cutting through the outer ring of hexagonal pins.

### Object

- [HexagonMeshTrimmer.md]
- (Cartesian sibling -- [CartesianMeshTrimmer.md])

### Geometry Features

- Two types of trimming can be performed by [HexagonMeshTrimmer.md]: Peripheral Trimming and Through-the-Center Trimming.

### Notes

- Peripheral trimming can be performed on six possible lines, each of which is parallel to a side of the hexagon and crosses the center of the pins laid out in that direction. Whether or not to trim a particular side of the hexagon is denoted by `1` (trim) or `0` (do not trim) in the 6-dimensional array [!param](/Mesh/HexagonMeshTrimmer/trim_peripheral_region).
- Peripheral trimming can only be used for assembly meshes
- Through-the-center trimming can be used for both assembly and core meshes. This mesh trimmer object RETAINS any sectors which are included between the trimming line defined by [!param](/Mesh/HexagonMeshTrimmer/center_trim_starting_index) to the trimming line defined by [!param](/Mesh/HexagonMeshTrimmer/center_trim_ending_index) swept out in a counterclockwise direction. Other sectors are discarded.
- When trimming along a line that lies exactly on element boundaries and does not cross any element interiors, an alternative mesh generator called [PlaneDeletionGenerator.md] can perform equivalent functionality
- When trimming along a line which crosses element interiors, [PlaneDeletionGenerator.md] leaves behind a zig-zag boundary, whereas these mesh generators smooth the trimmed boundary by moving nearby nodes to the trimmed line as needed. This may result in the creation of new triangular element blocks to avoid degenerate quadrilateral elements.

!media tutorial04_meshing/base_ex_hmt.png
       id=tutorial04-base_ex_hmt
       caption=Possible trimming lines of [HexagonMeshTrimmer.md].
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

### Peripheral Trimming Example

!media tutorial04_meshing/base_ex_peripheral_trim.png
       id=tutorial04-base_ex_peripheral_trim
       caption=Mesh peripheral trimming example.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-peripheral_trim.i
         caption=Mesh peripheral trimming example input.
         block=Mesh/pattern_assm_peri_trim

### Center Trimming Example

!media tutorial04_meshing/base_ex_center_trim.png
       id=tutorial04-base_ex_center_trim
       caption=Mesh center trimming example.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-center_trim.i
         caption=Mesh center trimming example input.
         block=Mesh/core_trim

## Assembly Periphery Modification

Assemblies with identical pin numbers and discretizations can be easily stitched together using [PatternedHexMeshGenerator.md]. However, real reactor cores often contain assemblies of various types which have different numbers of pins (e.g., control assembly and fuel assembly). When the number of pins in neighboring assemblies does not match, the outer boundaries of these neighboring assemblies will in general not match each other unless the user increases the azimuthal discretization of each mesh until the two edges have the same node count. This procedure is undesirable as it can excessively refine the mesh beyond what is needed. [PatternedHexPeripheralModifier.md] is available to modify the boundary nodes of an existing assembly mesh to enforce a specific number of nodes (uniformly distributed) to facilitate stitching to neighboring assemblies. The mesh generator does this by changing the outermost layer of elements into a transition layer which either increases or decreases the number of nodes on the outer boundary.

### Object

- [PatternedHexPeripheralModifier.md]
- (Cartesian sibling -- [PatternedCartesianPeripheralModifier.md])

### Geometry Features

- Modify the peripheral region of an assembly mesh to enforce a given number of nodes uniformly distributed on the external boundary to facilitate the stitching of different assembly meshes.

### Notes

- The input mesh must be an assembly with a hexagonal boundary (coolant and/or duct region(s) present).

### Example

!media tutorial04_meshing/base_ex_phpm.png
       id=tutorial04-base_ex_phpm
       caption=An example assembly mesh with its peripheral region modified by [PatternedHexPeripheralModifier.md].
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-pattern_assm_peri_mod.i
         caption=Periphery modification example input.
         block=Mesh/pattern_assm_peri_mod

## Extrusion to 3D

Many reactor geometries can be accurately described with extruded geometry in the axial direction. Extrusion from a 2D mesh to 3D is a very common mesh operation handled by [AdvancedExtruderGenerator.md]. The height of each layer, meshing control, subdomain IDs, and extra element integers can be specified by the user in this mesh generator to effectively generate a 3D mesh from a 2D one.

### Object

- [AdvancedExtruderGenerator.md]

### Geometry Features

- Extrudes a 1D mesh into 2D, or a 2D mesh into 3D
- Can have a variable height for each elevation
- Variable number of layers within each elevation
- Variable growth factors of axial element sizes within each elevation
- Remap subdomain IDs, boundary IDs and element extra integers within each elevation as well as interface boundaries between neighboring elevation layers.

### Notes

- Extrusion may be performed along any direction specified by an $(x,y,z)$ vector. Most common vector is $(0,0,1)$ which is the +$z$-direction.

### Example

!media tutorial04_meshing/base_ex_aeg.png
       id=tutorial04-base_ex_aeg
       caption=An example extruded mesh generated by [AdvancedExtruderGenerator.md] with subdomains swapped.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-common-core_ext.i
         caption=Advanced extrusion example input.
         block=Mesh/core_ext

!content pagination previous=tutorial04_meshing/step05_common_geom.md
                    next=tutorial04_meshing/step07_extra_element_ids.md
