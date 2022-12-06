[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []

  # Rename parameters supplied through the "tests" specifications
  [rename]
    type = RenameBoundaryGenerator
    input = gmg
  []

  # We compare by element numbers, which are not consistent in parallel
  # if this is true
  allow_renumbering = false
[]

[Reporters/mesh_info]
  type = MeshInfo
  items = sideset_elems
[]

[Outputs]
  [out]
    type = JSON
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
