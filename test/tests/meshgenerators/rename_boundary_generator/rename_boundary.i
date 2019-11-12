[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []

  # Rename parameters supplied through the "tests" specifications
  [./rename]
    type = RenameBoundaryGenerator
    input = gmg
  []
[]

[Outputs]
  exodus = true
[]
