[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = twoblocks.e
  []

  [./extrude]
    type = SideSetsBetweenSubdomainsGenerator
    input = fmg
    master_block = 'left'
    paired_block = 'right'
    new_boundary = 'in_between'
  [../]
[]

[Outputs]
  exodus = true
[]
