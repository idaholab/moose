[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = generate_distributed_exodus_in.e
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
