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
  []
  [cut_one]
    type = CartesianMeshGenerator
    dim = 2
    dx = 1
    dy = 1
    ix = 2
    iy = 2
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'big_one cut_one'
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
