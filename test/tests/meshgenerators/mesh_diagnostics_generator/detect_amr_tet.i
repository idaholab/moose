[Mesh]
  [big_one]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 1
    xmax = 2
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    elem_type = TET4
  []
  [cut_one]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    elem_type = TET4
  []
  [refined]
    type = RefineBlockGenerator
    input = cut_one
    refinement = 1
    block = 0
  []
  [cmbn]
    type = StitchedMeshGenerator
    inputs = 'big_one refined'
    stitch_boundaries_pairs = 'left right'
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = cmbn
    search_for_adaptivity_nonconformality = INFO
  []
[]

[Outputs]
  exodus = true
[]
