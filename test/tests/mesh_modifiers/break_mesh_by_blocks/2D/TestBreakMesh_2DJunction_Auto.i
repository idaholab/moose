[Mesh]
  file = 4ElementJunction.e
  parallel_type = REPLICATED
[]

[MeshModifiers]
  [./breakmesh]
    type = BreakMeshByBlock
  [../]
[]
