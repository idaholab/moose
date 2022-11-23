[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [boundary_removal]
    type = BoundaryDeletionGenerator
    input = gmg
    boundary_names = 'right top'
  []
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
