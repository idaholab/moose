# Mesh System

## Overview

There are two primary ways of creating a mesh for use in a MOOSE simulation: "offline generation" through
a tool like [CUBIT](https://cubit.sandia.gov/) from [Sandia National Laboratories](http://www.sandia.gov/), and
"online generation" through programmatic interfaces. CUBIT is useful for creating complex geometries, and can be
licensed from Coreform for a fee depending on the type of organization and work
being performed. Other mesh generators can work as long as they output a file format that is
supported by the [FileMesh](/FileMesh.md) object.

## Example Syntax and Mesh Objects

Mesh settings are applied with the `[Mesh]` section in input files, for example the basic input file
syntax for generating a simple square mesh is shown below. For additional information on the other types
of Mesh objects refer to the individual object pages listed below.

!listing test/tests/auxkernels/solution_aux/build.i block=Mesh

!syntax list /Mesh objects=True actions=False subsystems=False

!syntax list /Mesh objects=False actions=False subsystems=True

!syntax list /Mesh objects=False actions=True subsystems=False

## MeshGenerator System

The MeshGenerator System is useful for programmatically constructing a mesh. This includes generating the mesh
from a serious of points and connectivity, adding features on the fly, linearly transforming the mesh, stitching
together pieces of meshes, etc. There are several built-in generators but this system is also extendable. MeshGenerators
may or may not consumer the output from other generators and produce a single mesh. They can be chained together
through dependencies so that complex meshes may be built up from a series of simple processes.

### Mesh Generator development

Mesh generator developers should call `mesh->set_isnt_prepared()` at the end of
the `generate` routine unless they are confident that their mesh is indeed
prepared. Examples of actions that render the mesh unprepared are

- Translating, rotating, or scaling the mesh. This will conceptually change the
  mesh bounding box, invalidate the point locator, and potentially change the
  spatial dimension of the mesh (e.g. rotating a line from the x-axis into the
  xy plane, etc.)
- Adding elements. These elements will need their neighbor links set in order
  for things like finite volume to work
- Changing element subdomains. This will invalidate the mesh subdomain cached
  data on the `libMesh::MeshBase` object
- Changing boundary IDs. This invalidates global data (e.g. data aggregated
  across all processes) in the `libMesh::BoundaryInfo` object

When in doubt, the mesh is likely not prepared. Calling `set_isnt_prepared` is a
defensive action that at worst will incur an unnecessary `prepare_for_use`,
which may slow down the simulation setup, and at best may save follow-on mesh
generators or simulation execution from undesirable behavior.

### DAG and final mesh selection id=final

When chaining together several MeshGenerators, you are implicitly creating a DAG (directed acyclic graph).
MOOSE evaluates and generates the individual objects to build up your final mesh. If your input file has
multiple end points, (e.g. B->A and C->A) then MOOSE will issue an error and terminate. Generally, it doesn't
make sense to have multiple end points since the output of one would simply be discarded anyway. It is possible
to force the selection of a particular end point by using the [!param](/Mesh/MeshGeneratorMesh/final_generator)
parameter in the Mesh block. This parameter can be used on any generator whether there is ambiguity or not in the generator dependencies.


## Outputting The Mesh

Since MOOSE contains a lot of ability to read/generate/modify meshes - it's often useful to be able to run all of
the Mesh related portions of the input file and then output the mesh.  This mesh can then be viewed (such as
with Peacock) or used in other MOOSE input files for further combination/modification.

This can be achieved by using the command line option `--mesh-only`.  By default `--mesh-only` will write a
mesh file with `_in.e` (the opposite of the `_out.e` that is appended from the output system)
appended to the input file name.  You can also optionally provide a mesh filename to
write out using `--mesh-only output_file.e`. When using the `--mesh-only` option, by default any extra element integers
defined on the mesh will also be outputted to the output Exodus file. To prevent extra element ids from being
output, the parameter `output_extra_element_ids` should be set to `false` in the `[Outputs]` block of the
input file as shown below:

```
[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = false
  []
[]
```

Alternatively, if only a subset of extra element ids should be outputted to the Exodus file, the parameter
`extra_element_ids_to_output` should be set in the `[Outputs]` block of the input file like so:

```
[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = true
    extra_element_ids_to_output = 'id_to_output1 id_to_output2 ...'
  []
[]
```

Here are a couple of examples showing the usage of `--mesh-only`:

```
# Will run all mesh related sections in input_file.i and write out input_file_in.e
./myapp-opt -i input_file.i --mesh-only

# Will do the same but write out mesh_file.e
./myapp-opt -i input_file.i --mesh-only mesh_file.e

# Run in parallel and write out parallel checkpoint format (which can be read as a split)
mpiexec -n 3 ./myapp-opt -i input_file.i Mesh/parallel_type=distributed --mesh-only mesh_file.cpr
```

## Named Entity Support

Human-readable names can be assigned to blocks, sidesets, and nodesets. These names will be
automatically read in and can be used throughout the input file. Mesh generators such as CUBIT will
generally provide the capability internally.  Any parameter that takes entity IDs in the input file
will accept either numbers or "names". Names can also be assigned to IDs on-the-fly in existing
meshes to ease input file maintenance (see example). On-the-fly names will also be written to
Exodus/XDA/XDR files. An illustration for mesh in exodus file format.

!listing test/tests/mesh/named_entities/name_on_the_fly.i block=Mesh

## Replicated and Distributed Mesh id=replicated-and-distributed-mesh

The core of the mesh capabilities are derived from [libMesh], which has two underlying
parallel mesh formats: "replicated" and "distributed".

The replicated mesh format is the default format for MOOSE and is the most appropriate format to
utilize for nearly all simulations. In parallel, the replicated format copies the complete mesh to
all processors allowing for efficient access to the geometry elements.

The distributed mesh format is useful when the mesh data structure dominates memory usage. Only the
pieces of the mesh "owned" by a processor are actually stored on the processor. If the mesh is too
large to read in on a single processor, it can be split prior to the simulation.

!alert note
Both the "replicated" and "distributed" mesh formats are parallel with respect to the execution of
the finite element assembly and solve. In both types the solution data is distributed, which is
the portion of the simulation that usually dominates memory demands.

### Distributed Mesh Output Format (Nemesis)

When running a simulation with `DistributedMesh` it is generally desirable to avoid serializing
the mesh to the first rank for output. In the largest case this may cause your simulation to run
out of memory, in smaller cases, it may just cause unnecessary communication to serialize your
parallel data structure. The solution is to use "nemesis" output.

Nemesis creates separate Exodus files that are automatically read by Paraview and displayed as
if a normal Exodus mesh had been output. The output files have the following naming convention:

```
<filename>.e.<num_processors>.<rank>

# For example, on a 4 processor run, you can expect filenames like this:
out.e.4.0
out.e.4.1
out.e.4.2
out.e.4.3
```

## Mesh splitting

For large meshes, MOOSE provides the ability to pre-split a mesh for use in the "distributed"
format/mode. To split and use a mesh for distributed runs:

```
// For input files with a file-based mesh:
$ moose-app-opt -i your_input-file.i --split-mesh 500,1000,2000 // comma-separated list of split configurations
Splitting 500 ways...
    - writing 500 files per process...
Splitting 1000 ways...
    - writing 1000 files per process...
...

// MOOSE automatically selects the pre-split mesh configuration based on MPI processes
$ mpiexec -n 1000 moose-app-opt -i your_input-file.i --use-split
```

For more details see "[Mesh Splitting](/Mesh/splitting.md)".

## Displaced Mesh

Calculations can take place in either the initial mesh configuration or, when requested, the
"displaced" configuration. To enable displacements, provide a vector of displacement variable names
for each spatial dimension in the 'displacements' parameters within the Mesh block.

!listing modules/solid_mechanics/test/tests/truss/truss_2d.i block=Mesh

Once enabled, the any object that should operate on the displaced configuration should set the
"use_displaced_mesh" to true. For example, the following snippet enables the computation of a
[Postprocessor](/Postprocessors/index.md) with and without the displaced configuration.

!listing test/tests/postprocessors/displaced_mesh/elemental.i block=Postprocessors

## Mixed Dimension Meshes

MOOSE will function properly when running simulations on meshes containing mixed dimension elements
(e.g. 1D and 2D, 1D and 3D, etc.). Residual calculation, material evaluation, etc should all work properly.

!listing test/tests/mesh/mixed_dim/1d_3d.i block=Mesh

## Unique IDs

There are two "first-class" id types for each mesh entity (elements or nodes): "id and unique_id". Both the id
and unique_id field are unique numbers for the current active set of mesh entities. Active entities are those
that are currently representing the domain but doesn't include "coarse parents" of some elements that may become
active during a coarsening step. The difference however is that unique_ids are never reused, but ids +might+ be.
Generally the id is "good-enough" for almost all use, but if you need guarantees that an element id is never
recycled (because it might be a key to an important map), you should use unique_id.

## Periodic Node Map

The MooseMesh object has a method for building a map (technically a multimap) of paired periodic nodes in the
simulation. This map provides a quick lookup of all paired nodes on a periodic boundary. in the 2D and 3D cases
each corner node will map to 2 or 3 other nodes (respectively).

## Extra integer IDs

Extra integer IDs for all the elements of a mesh can be useful for handling complicated material assignment, performing specific calculations on groups of elements, etc.
Often times, we do not want to use subdomain IDs for these tasks because otherwise too many subdomains could be needed, and in turn large penalty on run-time performance could be introduced.

MooseMesh[MooseMesh.md] has a parameter `extra_integers` to allow users to introduce more integer IDs for elements each identified with a name in the parameter.
When this parameter is specified, extra integers will be made available for all elements through `Assembly` in MOOSE objects such as kernels, aux kernels, materials, initial conditions, element user objects, etc.
To retrieve the integer on an element, one needs to simply call

```
getElementID(integer_name_parameter, comp),
```

within the initialization list of your constructor.
`integer_name_parameter` is the name of the parameter in type of `std::vector<ExtraElementIDName>` of this object listing all integer names.
`comp` is the index into the integer names if multiple are specified for `integer_name_parameter`.
It is noticed that the returned value of this function call must be in type of `const dof_id_type &`, which is used to refer the value set by MOOSE in `Assembly`.
The returned reference should be held in a class member variable for later use.
Based on this ID, one can proceed with any particular operations, for example, choosing a different set of data for evaluating material properties.

IDs can be assigned to the mesh elements with `MeshGenerators` in a similar way to assigning subdomain IDs.
We note that the element IDs are part of the mesh and will be initialized properly for restart/recover.

## Mesh meta data

Mesh generators can declare mesh meta data, which can be obtained later in Actions or in UserObjects.
Mesh meta data can only be declared in the constructors of mesh generators so that they can be restarted without re-running mesh generators.
Mesh meta data can be useful for setting up specific postprocessors, kernels, etc. that require certain geometry information.
Mesh meta data are not possible or extremely hard to be derived directly from libMesh mesh object.
A simple example of mesh meta data is the `num_elements_x` provided by [GeneratedMeshGenerator](GeneratedMeshGenerator.md), which can be used as an indicator for a mesh regular in x direction.

## Debugging in-MOOSE mesh generation id=troubleshooting

!alert note
The MOOSE mesh generation [tutorial](tutorial04_meshing/index.md optional=true) is the most comprehensive resource on learning how to mesh within MOOSE. We summarize here
only a few techniques.

Mesh generation in MOOSE is a sequential tree-based process. Mesh generators are executed sorted by dependencies,
and the output of each generator may be fed to multiple other generators. To succeed in this process, you must decompose
the creation of the mesh into many individual steps. To debug this process, one can:

- use the `show_info(=true)` input parameter on each mesh generator. This will output numerous pieces of metadata about the mesh
  at each stage of the generation process. You can check there if all the subdomains that you expected at this stage
  are present in the mesh and if they are of the expected size, both in terms of number of elements but also bounding box.
- use the `output` input parameter on the mesh generator right before the problematic stage. This will output the mesh,
  by default using the [Exodus.md] format with the name `<mesh_generator_name>_in.e`, so you may visualize it
  before it gets acted upon by the next mesh generator(s).

For a narrow selection of mesh issues, listed in its documentation, the [MeshDiagnosticsGenerator.md] may be used to detect
unsupported features in meshes.

## Examining meshes id=examination

The results of finite element/volume simulations are highly dependent on the quality of the mesh(es) used.
It happens regularly that results are excellent and meeting all predictions using a regular Cartesian grid mesh,
but significantly deteriorate or do not converge on the real system mesh, often created outside MOOSE.

We point out in this section a few things to look for.
- Sidesets in MOOSE are oriented. If you place a Neumann/flux boundary condition on a sideset, the direction of
  the flux will depend on the orientation of the sideset.
- MOOSE generally does not support non-conformal meshes for regular kernels, except when they arise from online mesh refinement.
  When inspecting your mesh, you should not see any hanging nodes or surfaces not exactly touching. If you are using such
  a mesh, you +MUST+ use interface kernels, mortar or other advanced numerical treatments.
- Many physics will give better results with high element quality and smooth distributions of element volumes.
  You may examine the spatial distribution of these quantities using the [ElementQualityAux.md] and [VolumeAux.md]
  respectively.

## Coordinate Systems id=coordinate_systems

The following are the coordinate systems currently available in MOOSE:

- `XYZ`: 3D Cartesian.
- `RZ`: 2D axisymmetric coordinates.
- `RSPHERICAL`: 1D spherical coordinates with the origin at $(0,0,0)$.

Coordinate systems may be specified in the input file or within code.

### Specifying coordinate systems in the input file

In an input file, coordinate systems may be specified in the [Mesh](Mesh/index.md)
block.
First, [!param](/Mesh/GeneratedMesh/coord_type) is used to specify the coordinate
system type. If you would like to use multiple coordinate systems in your
application, you can supply multiple entries in this parameter. Then you must
specify [!param](/Mesh/GeneratedMesh/coord_block) to specify the corresponding
blocks to which each coordinate system applies.

If the `RZ` coordinate system is used, there are two options for how to specify
the coordinate axis(es) in an input file:

- Specify [!param](/Mesh/GeneratedMesh/rz_coord_axis) to choose a single `RZ`
  coordinate system, using the $\hat{x}$ or $\hat{y}$ direction and starting at $(0,0,0)$.
  If the former is used, then the axial coordinate is $x$, and the radial coordinate
  is $y$; if the latter is used, these are switched.
- Specify the following three parameters:

  - [!param](/Mesh/GeneratedMesh/rz_coord_blocks): The list of blocks using an
    `RZ` coordinate system (all must be specified).
  - [!param](/Mesh/GeneratedMesh/rz_coord_origins): The list of origin points
    for the axisymmetric axes corresponding to each block in [!param](/Mesh/GeneratedMesh/rz_coord_blocks).
  - [!param](/Mesh/GeneratedMesh/rz_coord_directions): The list of direction vectors
    for the axisymmetric axes corresponding to each block in [!param](/Mesh/GeneratedMesh/rz_coord_blocks).
    Note that these direction vectors need not be unit vectors, just nonzero vectors.

The second option has greater flexibility, as it allows the following, which the
first option does not:

- Multiple axisymmetric coordinate systems can be defined.
- Any point can be used for the origin of the coordinate system, not just $(0,0,0)$.
- Any direction can be used for the axisymmetric axis, not just the $\hat{x}$ or $\hat{y}$ direction.

Note that the [Transfers](Transfers/index.md) ability for the second option is
more limited

### Specifying coordinate systems within code

To specify coordinate systems within code, `MooseMesh::setCoordSystem(blocks, coord_sys)` is used,
where `blocks` and `coord_sys` have the same behavior as the [!param](/Mesh/GeneratedMesh/coord_block)
and [!param](/Mesh/GeneratedMesh/coord_type) parameters, respectively.

If the `RZ` coordinate system is used, there are two options for how to specify
the coordinate axis(es) within the code, just like in the input file:

- Call `MooseMesh::setAxisymmetricCoordAxis(rz_coord_axis)`, where
  `rz_coord_axis` is like [!param](/Mesh/GeneratedMesh/rz_coord_axis).
- Call `MooseMesh::setGeneralAxisymmetricCoordAxes(blocks, axes)`, where
  `blocks` is similar to [!param](/Mesh/GeneratedMesh/rz_coord_blocks)
  and `axes` pairs up the origins and directions, similar to combining the parameters
  [!param](/Mesh/GeneratedMesh/rz_coord_origins) and [!param](/Mesh/GeneratedMesh/rz_coord_directions).
