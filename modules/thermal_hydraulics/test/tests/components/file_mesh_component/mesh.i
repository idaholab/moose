# This file generates the mesh for the FileMeshComponent test.

[Mesh]
  [gen_mesh_mg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 2
    xmin = 0
    xmax = 5.0
    ymin = 2.0
    ymax = 3.0
  []
  [rename_block_mg]
    type = RenameBlockGenerator
    input = gen_mesh_mg
    old_block = 0
    new_block = 'block_a'
  []
[]
