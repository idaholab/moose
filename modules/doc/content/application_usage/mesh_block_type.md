# Mesh

## Creating a Mesh

- For complicated geometries, we generally use CUBIT from Sandia National Laboratories.
  - Cubit® is available for US government use only and requires proof of a current US government contract. 
- If your work is not US government use, you can use [Coreform Cubit](https://coreform.com/coreform-cubit/), the commercial version of Cubit®, available through [Coreform](https://coreform.com/), which is available for purchase for any use, including academic and commercial.
  - Coreform also provides a free non-commercial-use "[Coreform Cubit Learn](https://coreform.com/coreform-cubit-learn/)" license, fully-functional with a 50k element export limit.
- Other mesh generators can work as long as they output a file format that libMesh reads (next section).
- If you have a specific mesh format you like, we can take a look at adding support for it to libMesh.

### FileMesh

- `FileMesh` is the default type.
- MOOSE supports reading and writing a large number of formats and could be extended to read more.

| Extension   | Description                              |
| :-          | :-                                       |
| .dat        | [Tecplot ASCII file](https://people.sc.fsu.edu/~jburkardt/data/tec/tec.html)                       |
| .e, .exd    | [Sandia's ExodusII format](https://sandialabs.github.io/seacas-docs/sphinx/html/index.html#exodus-library)                 |
| .fro        | [ACDL's surface triangulation file](https://www.reviversoft.com/file-extensions/fro)        |
| .gmv        | [LANL's GMV (General Mesh Viewer) format](www.generalmeshviewer.com/)  |
| .mat        | [Matlab triangular ASCII file](https://www.reviversoft.com/file-extensions/mat) (read only) |
| .msh        | [GMSH ASCII file](http://www.manpagez.com/info/gmsh/gmsh-2.2.6/gmsh_63.php)                          |
| .n, .nem    | [Sandia's Nemesis format](https://gsjaardema.github.io/seacas/)                  |
| .plt        | [Tecplot binary file](http://home.ustc.edu.cn/~cbq/360_data_format_guide.pdf) (write only)         |
| .node, .ele; .poly | [TetGen ASCII file](http://wias-berlin.de/software/tetgen/1.5/doc/manual/manual006.html) (read; write)   |
| .inp        | [Abaqus .inp format](https://www.sharcnet.ca/Software/Abaqus610/Documentation/docs/v6.10/books/usb/default.htm?startat=pt01ch03s06aus34.html) (read only)           |
| .ucd        | [AVS's ASCII UCD format](http://people.sc.fsu.edu/~jburkardt/data/ucd/ucd.html)                   |
| .unv        | [I-deas Universal format](https://knowledge.autodesk.com/support/moldflow-insight/learn-explore/caas/CloudHelp/cloudhelp/2018/ENU/MoldflowInsight/files/GUID-03956F5F-D1C7-4E75-99ED-73F1E2ECF225-htm.html)                  |
| .xda, .xdr  | [libMesh formats](https://sourceforge.net/p/libmesh/mailman/attachment/AD00A4C1B366594CB4B07B0A970306A90990AF91%40jsc-mail03.jsc.nasa.gov/1/)                          |
| .vtk, .pvtu | [Visualization Toolkit](https://www.vtk.org/wp-content/uploads/2015/04/file-formats.pdf)                    |

[](---)

### GeneratedMesh

- `type = GeneratedMesh`
- Built-in mesh generation is implemented for lines, rectangles, and rectangular prisms ("boxes") but could be extended.
- The sides are named in a logical way and are numbered:

  - In 1D, left = 0, right = 1
  - In 2D, bottom = 0, right = 1, top = 2, left = 3
  - In 3D, back = 0, bottom = 1, right = 2, top = 3, left = 4, front = 5

- The length, width and height of the domain, and the number of elements in each direction can be specified independently.

## Named Entity Support

- Human-readable names can be assigned to blocks, sidesets, and nodesets.
- These names will be automatically read in and can be used throughout the input file. This is typically done inside of CUBIT.
- Any parameter that takes entity IDs in the input file will accept either numbers or "names".
- Names can also be assigned to IDs on-the-fly in existing meshes to ease input file maintenance (see example).
- On-the-fly names will also be written to Exodus/XDA/XDR files.
- An illustration for mesh in Exodus file format :

  ```puppet
  [Mesh]
  file = three_block.e

  #These names will be applied on the
  #fly to the mesh so that they can be
  #used in the input file. In addition
  #they will be written to the output
  #file
  block_id = '1 2 3'
  block_name = 'wood steel copper'

  boundary_id = '1 2'
  boundary_name = 'left right'
  []
  ```

[](---)

## Example Name Support

An illustration for mesh in UNV file format for having names for blocks and boundaries.

```puppet
[Mesh]
  file = three_block.unv
  #If the names wood, steel, copper, left and right
  #are generated via Salome software in the UNV mesh
  #then they can be read directly by MOOSE
  #which is illustrated here by commenting(#) the numbers
  #block_id = '1 2 3'
  block_name = 'wood steel copper'
  #boundary_id = '1 2'
  boundary_name = 'left right'
[]
...
[BCs]
  [./left_bc] #Temperature on left edge is fixed at 800K
    type = DirichletBC
    preset = true
    variable = T
    #boundary = '1'
    boundary = 'left'
    value = 800
  [../]
  [./right_x] #Temperature in the right sideset is fixed at 298 K
    type = DirichletBC
    variable = T
    #boundary = '2'
    boundary = 'right'
    value = 298.0
  [../]
[]
```

An illustration for mesh in UNV file format and requirement of on-the-fly names of block and boundaries in moose input file.

```puppet
[Mesh]
  file = three_block.unv
  #the numbers should be verified from the
  #unv mesh format to be represented for a
  #particular block and boundary of the geometry
  #The random BC ids 7 and 48 are written here
  #to highlight the role of verification from within
  #the file
  #That is the numbers 1, 2 ,3 , 7 and 48 are obtained from the mesh
  block_id = '1 2 3'
  block_name = 'wood steel copper'
  boundary_id = '7 48'
  boundary_name = 'left right'
[]
...
[BCs]
  [./left_bc] #Temperature on left edge is fixed at 800K
    type = DirichletBC
    preset = true
    variable = T
    #boundary = '7'
    boundary = 'left'
    value = 800
  [../]
  [./right_x] #Temperature in the right sideset is fixed at 298 K
    type = DirichletBC
    variable = T
    #boundary = '48'
    boundary = 'right'
    value = 298.0
  [../]
[]
```

[](---)

## Parallel Mesh

- Useful when the mesh data structure dominates memory usage.
- Only the pieces of the mesh "owned" by processor *N* are actually stored by processor *N*.
- If the mesh is too large to read in on a single processor, it can be split prior to the simulation.
- Copy the mesh to a large memory machine.
- Use a tool to split the mesh into *n* pieces (SEACAS, loadbal).
- Copy the pieces to your working directory on the cluster.
- Use the Nemesis reader to read the mesh using *n* processors.

## Displaced Mesh

- Calculation can take place in either the initial mesh configuration or, when requested, the "displaced" configuration.
- To enable displacements, provide a vector of displacement variable names for each spatial dimension in the section, e.g.:

  ```text
  displacements = 'disp_x disp_y disp_z'
  ```

- Once enabled, the parameter can be set on individual MooseObjects which will enable them to use displaced coordinates during calculations:

  ```cpp
  InputParameters
  SomeKernel::validParams()
  {
    InputParameters params = Kernel::validParams();
    params.set<bool>("use_displaced_mesh") = true;
    return params;
  }
  ```

- This can also be set in the input file, but it is a good idea to do it in the code if you have a pure Lagrangian formulation.
