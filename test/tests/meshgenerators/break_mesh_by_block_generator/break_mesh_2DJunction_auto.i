[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = 4ElementJunction.e
  []

  [./breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
  []
[]

[Outputs]
  exodus = true
[]
