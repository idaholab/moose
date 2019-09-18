[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = 4ElementJunction.e
  []

  [./breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
    split_interface = true
  []
[]

[Outputs]
  exodus = true
[]
