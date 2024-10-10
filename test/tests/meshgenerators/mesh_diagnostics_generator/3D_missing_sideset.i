[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
  [deletion]
    type = BoundaryDeletionGenerator
    input = gmg
    boundary_names = 'right'
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = deletion
    check_for_watertight_sidesets = INFO
  []
[]

[Outputs]
  exodus = true
[]
