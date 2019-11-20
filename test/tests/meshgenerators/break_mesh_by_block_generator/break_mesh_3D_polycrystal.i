[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = poly2.msh
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
