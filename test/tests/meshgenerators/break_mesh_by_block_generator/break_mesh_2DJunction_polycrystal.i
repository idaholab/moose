[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = poly.msh
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
