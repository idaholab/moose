[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
  [checkpoint]
    type = FileMeshGenerator
    file = checkpoint_old_prefix_out_cp/0001_mesh.cpr
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
