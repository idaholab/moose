[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  [./rename]
    type = RenameBoundaryGenerator
    input = gmg
    old_boundary_id = '0 1 10'
    new_boundary_id = '10 0 1'
  []
[]

[Outputs]
  exodus = true
[]
