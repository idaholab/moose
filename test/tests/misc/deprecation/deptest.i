[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []

  [rename]
    type = RenameBoundaryGenerator
    input = gmg
    old_boundary_id = '0 1 3 2'
    new_boundary_id = '1 2 4 3'
  []
[]

[Reporters/mesh_info]
  type = MeshInfo
  items = sideset_elems
[]

[Outputs]
  [out]
    type = JSON
    #execute_system_information = NONE
  []
[]
[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
