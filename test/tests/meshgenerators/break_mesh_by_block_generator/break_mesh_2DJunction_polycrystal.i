[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = poly.msh
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
