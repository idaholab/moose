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
  []
  [cut_one]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 1
    ymax = 1
    zmax = 1
    nx = 2
    ny = 2
    nz = 2
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
