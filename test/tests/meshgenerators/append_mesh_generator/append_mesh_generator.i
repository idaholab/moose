[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
  []
[]

[ModifyMesh]
  [addss]
    type = SideSetsAroundSubdomainGenerator
    new_boundary = whole
    block = 0
  []
[]

[Outputs]
  exodus = true
[]
