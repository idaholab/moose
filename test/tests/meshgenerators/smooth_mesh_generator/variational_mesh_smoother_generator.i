[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = concentric_circle_mesh_in.e
  []

  [./smooth]
    type = VariationalSmoothMeshGenerator
    input = fmg
    iterations = 5
  []
[]

[Outputs]
  exodus = true
[]
