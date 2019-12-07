[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = testTri10.e
  []
[]

[Outputs]
  exodus = true
[]

# to replicate run this with --distributed-mesh
