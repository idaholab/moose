# This input file is used to generate a rectangle mesh for other tests. It
# should be run with "--mesh-only rectangle.e".

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 1
    nx = 20
    ny = 5
  []
  [rename_block]
    type = RenameBlockGenerator
    input = gen
    old_block = 0
    new_block = 'body'
  []
[]
