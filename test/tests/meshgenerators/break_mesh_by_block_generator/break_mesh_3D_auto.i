[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = coh3D_3Blocks.e
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
