[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = concentric_circle_mesh_in.e
  []

  [./smooth]
    type = SmoothMeshGenerator
    algorithm = laplace
    input = fmg
    iterations = 3
  []
[]

[Outputs]
  exodus = true
[]
