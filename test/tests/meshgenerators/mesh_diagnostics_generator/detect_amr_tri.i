[Mesh]
  [big_one]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin = 1
    xmax = 2
    ymin = 0
    ymax = 1
    elem_type = TRI3
  []
  [cut_one]
    type = GeneratedMeshGenerator
    dim = 2
    # Actually looks the same as uniformly refined
    nx = 2
    ny = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    elem_type = TRI3
  []
  [cmbn]
    type = StitchedMeshGenerator
    inputs = 'big_one cut_one'
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
