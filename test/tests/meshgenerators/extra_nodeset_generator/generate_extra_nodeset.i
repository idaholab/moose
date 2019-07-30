[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = square.e
  []

  [./extra_nodeset]
    type = ExtraNodesetGenerator
    input = fmg
    new_boundary = 'middle_node'
    nodes = '2'
  []
[]

[Outputs]
  exodus = true
[]
