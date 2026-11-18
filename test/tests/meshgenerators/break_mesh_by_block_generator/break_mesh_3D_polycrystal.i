[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = poly2.msh
    #parallel_type = replicated
  []

  [./breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
  []
[]

[Outputs]
  exodus = true
[]
