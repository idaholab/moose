[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
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
