# Mesh Generation

!---

## BISON Input Files

- BISON requires two files in order to run.

- The first of these is an input text file.

- The second is an input mesh file.

  - The default format is ExodusII [!citep](exodus:96).

- The creation of the mesh file is the subject of this section.

  - Meshes can be created external to the input text file.

  - Meshes can be created internal to the input text file.

!---

## Creating Mesh Input Files

- [CUBIT](https://cubit.sandia.gov/) from Sandia National Laboratories [!citep](cubit).

  - Use CUBIT directly.

  - Use scripts to drive CUBIT.

- Create an Abaqus file and import that into BISON instead.

- Output ExodusII from Patran or Ansys.

CUBIT can be licensed from Sandia (free for government use). A commercial version,
Trelis, is available from [csimsoft](https://www.csimsoft.com/trelis).

!---

## CUBIT Interface

!style halign=center
!media cubit.png style=width:90%

!---

## CUBIT Capabilities

- Generating a solid model

- Importing a solid model

- Automatically generating a mesh for simple geometries

- Creating 1D, 2D, or 3D meshes

- Assigning blocks, side sets, and node sets

- Being driven by a GUI, command line, journal file, or Python

!---

## BISON's Mesh Generation Scripts

!row!
!col! width=60%

- Shell and Python scripts for mostly automatic fuel rod mesh
  generation are in `bison/tools/UO2`.

- Relevant files are:

  - `mesh_script.sh`: Sets up environment variables. Calls `mesh_script.py` and `mesh_script_input.py`.

  - `mesh_script.py`: Main script. Interfaces with CUBIT. Handles both
    2D and 3D geometries. User should not have to modify this file.

  - `mesh_script_input.py` Input file. Defines geometry and mesh
    parameters using Python dictionaries.

!col-end!

!col! width=40%
!col width=40%

!style halign=center
!media medium3D.png style=width:55%;

!col-end!

!row-end!

!---

## Mesh input file review: Fuel

In the following slides, we will go over the options available in
the `mesh_script_input.py` file.

!row!
!col! width=48%
!col width=48%

!listing workshop/mesh_script_example/mesh_script_input.py  include-end=true
         start=Pellet Type 1:  end=Pellet1['chamfer_height']
         style=width:80%

!col-end!

!col! width=4%
!col width=4%

$~$

!col-end!

!col! width=48%
!col width=48%

!listing workshop/mesh_script_example/mesh_script_input.py  include-end=true
         start=Pellet Type 2:  end=Pellet2['chamfer_height']
         style=width:80%

!col-end!

!row-end!

!---

## Mesh input file review: Pellet stack


!listing workshop/mesh_script_example/mesh_script_input.py  include-end=true
         start=Pellet Collection   end=pellet_stack['angle']

!row!

!col! width=50%
!col width=50%

- `default_parameters` use default parameters without considering
  below parameters
- `interface_merge`

  - `point` (default) common vertex (2D) or curve (3D)
  - `none` not merged

!col-end!

!col! width=50%
!col width=50%

- `higher_order`

  - `False`: QUAD4 (2D) or HEX8 (3D).
  - `True`: QUAD8 (2D) or HEX27 (3D).

- `angle`

  - 0: create a 2D-RZ geometry.
  - $>$ 0: create a 3D stack of the specified angle ($\leq 360^\circ$)

!col-end!

!row-end!


!---

## Mesh input file review: Clad

Defines clad geometric parameters. Please note:

- `mesh_density`: clad mesh depends on fuel mesh.

- `clad_width`: this parameter is the total width of the clad, +including the liner+.


!listing workshop/mesh_script_example/mesh_script_input.py  include-end=true
         start=Clad:   end=clad['liner_width']

!---

## Mesh input file review: Meshing parameters

!row!


- Mesh parameters are also stored in a dictionary.

- The name of the dictionary must be the same as defined in the pellet type block (`mesh_density`).

!row-end!

!row!

!col! width=33%
!col width=33%

!listing workshop/mesh_script_example/mesh_script_input.py  include-end=true
         start=Meshing   end=medium['clad_angular_interval']
         style=width:95%

!col-end!

!col! width=67%
!col width=67%


- For a smeared pellet, the mesh density of the fuel is controlled by
  the parameters `pellet_r_interval` and `pellet_z_interval`. Other
  `pellet*` parameters are used with a discrete geometry.

- `clad_sleeve_scale_factor`

  - 1: same vertical density as the fuel

  - $> 1$: higher density

  - $< 1$: smaller density

  - Recommend that $\le 1$

!col-end!
!row-end!

!---

## Output review: Boundary conditions

!style halign=center
!media mesh_schematic.png

!---

## 3D Boundary Conditions (180-degree model)

!row!

!col! width=50%
!col width=50%

!style halign=center fontsize=80%
+3D Mesh+

!style halign=center
!media Discrete_One_Pellet_base.png style=width:75%

!col-end!

!col! width=50%
!col width=50%


!style halign=center fontsize=80%
+Sideset 99 Definition+

!style halign=center
!media Discrete_One_Pellet_ss99.png style=width:75%

!col-end!

!row-end!

!---

## 3D Boundary Conditions (90-degree model)

!row!

!col! width=32%
!col width=32%

!style halign=center fontsize=80%
+3D Mesh+

!style halign=center
!media Discrete_One_Pellet_90deg_base.png style=width:90%

!col-end!

!col! width=2%
!col width=2%

$~$

!col-end!

!col! width=32%
!col width=32%

!style halign=center fontsize=80%
+Sideset 98 Definition+

!style halign=center
!media Discrete_One_Pellet_90deg_ss98.png style=width:90%

!col-end!

!col! width=2%
!col width=2%

$~$

!col-end!

!col! width=32%
!col width=32%

!style halign=center fontsize=80%
+Sideset 99 Definition+

!style halign=center
!media Discrete_One_Pellet_90deg_ss99.png style=width:90%

!col-end!

!row-end!

!---

## Mesh script: Wrap-up

!row!
!col! width=50%
!col width=50%

- Geometry and mesh parameters are defined in the input file for 2D or 3D geometry

- No interaction with the main script is required

- In the Exodus file, blocks have these names: `clad`, `liner`, and `pellet_type_#`

!col-end!

!col! width=50%
!col width=50%

!media MeshArchitecture.png

!col-end!
!row-end!

!---

## Mesh Examples

!row!

!col! width=24.25%
!col width=24.25%

!style halign=center fontsize=80%
+Coarse+

!style halign=center
!media coarse.png style=width:55%

!col-end!

!col! width=1%
!col width=1%

$~$

!col-end!

!col! width=24.25%
!col width=24.25%

!style halign=center fontsize=80%
+Medium+

!style halign=center
!media medium.png style=width:55%

!col-end!

!col! width=1%
!col width=1%

$~$

!col-end!

!col! width=24.25%
!col width=24.25%

!style halign=center fontsize=80%
+Fine+

!style halign=center
!media fine.png style=width:55%

!col-end!

!col! width=1%
!col width=1%

$~$

!col-end!

!col! width=24.25%
!col width=24.25%

!style halign=center fontsize=80%
+3D Medium+

!style halign=center
!media medium3D.png style=width:80%

!col-end!
!row-end!

!---

## Mesh Generators

MOOSE and BISON also have internal meshing capabilities.  Capabilities specific
to BISON include:

- [SmearedPelletMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/SmearedPelletMeshGenerator.html)
- [Layered1DMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/Layered1DMeshGenerator.html)
- [CircularCrossSectionMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/CircularCrossSectionMeshGenerator.html)
- [MPSCircularCrossSectionMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/MPSCircularCrossSectionMeshGenerator.html)
- [Layered2DMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/Layered2DMeshGenerator.html)
- [PlateMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/PlateMeshGenerator.html)
- [TRISO1DFiveLayerMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/TRISO1DFiveLayerMeshGenerator.html)
- [TRISO1DMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/TRISO1DMeshGenerator.html)
- [TRISO2DMeshGenerator](https://mooseframework.org/bison/source/meshgenerators/TRISO2DMeshGenerator.html)

A complete list of all available MeshGenerators can be found [here](https://mooseframework.org/bison/syntax/MeshGenerators/index.html).

!---

## SmearedPelletMeshGenerator

Used to create 2D-RZ axisymmetric smeared pellet meshes.

!row!

!col! width=49%
!col width=49%

!listing test/tests/smeared_pellet_mesh_generator/smeared_pellet_mesh_with_coating.i block=Mesh

!col-end!

!col! width=2%
!col width=2%

$~$

!col-end!

!col! width=49%
!col width=49%

!style halign=center
!media smeared_pellet_mesh.png style=width:20%

!col-end!

!row-end!

!---

## Layered1DMeshGenerator

Used to create 2D-RZ axisymmetric layered 1D meshes.

!row!

!col! width=49%
!col width=49%

!listing assessment/LWR/validation/LOCA_IFA_650/analysis/IFA_650_9/IFA_650_9_part1.i block=Mesh

!col-end!

!col! width=2%
!col width=2%

$~$

!col-end!

!col! width=49%
!col width=49%

!style halign=center
!media schematic_Layered1DMesh.png
      style=position:relative;left:50px;

!col-end!

!row-end!

!---

## CircularCrossSectionMeshGenerator

Used to create 2D plane strain meshes.

!row!

!col! width=49%
!col width=49%

!listing test/tests/circular_cross_section_mesh/circular_cross_section_mesh_generator.i block=Mesh

!col-end!

!col! width=2%
!col width=2%

$~$

!col-end!

!col! width=49%
!col width=49%

!style halign=center
!media circular_cross_section_nodesets.png

!col-end!

!row-end!

!--

## MPSCircularCrossSectionMeshGenerator

Used to create 2D plane strain meshes containing an MPS.

!row!

!col! width=49%
!col width=49%

!listing test/tests/circular_cross_section_mesh/mps_circular_cross_section_mesh_generator.i block=Mesh

!col-end!

!col! width=2%
!col width=2%

$~$

!col-end!

!col! width=49%
!col width=49%

!style halign=center
!media mps_circular_cross_section_nodesets.png

!col-end!

!row-end!

!---

## Layered2DMeshGenerator

Used to create 2D plane strain layered 2D meshes.

!row!

!col! width=49%
!col width=49%

!listing test/tests/layered2D/layered2D_additional_blocks_mesh.i block=Mesh

!col-end!

!col! width=2%
!col width=2%

$~$

!col-end!

!col! width=49%
!col width=49%

!style halign=center
!media layered2D_nodeset_schematic.png

!col-end!

!row-end!

!---

## PlateMeshGenerator

Creates a 3D mesh of a nuclear fuel plate, including fuel, liner, and cladding.

!row!

!col! width=50%
!col width=50%

!listing test/tests/plate_mesh/plate_mesh.i block=Mesh

!col-end!

!col! width=50%
!col width=50%

!style halign=center
!media plate_mesh.png style=width:70%

!col-end!

!row-end!

!---

## TRISO1DMeshGenerator

Creates a 1D mesh of a TRISO fuel particle by specifying the radial coordinates
corresponding to the interface between different materials within the particle.

!listing examples/TRISO/full_particle/1D/full_particle_1D.i block=Mesh

!style halign=center
!media TRISO_1D_map.png style=width:50%

!---

## TRISO1DFiveLayerMeshGenerator

Creates a 1D mesh of a TRISO fuel particle by specifying the radius of the fuel
kernel and the thicknesses of the buffer, IPyC, SiC, and OPyC layers. This
mesh generator is primarily used for failure analyses.

!listing test/tests/triso_failure/triso_1d_ipyc_failure.i block=Mesh

!---

## TRISO2DMeshGenerator

Creates a 2D mesh of a TRISO fuel particle.

!listing test/tests/triso/mesh/mesh_with_gap_2D.i block=Mesh
