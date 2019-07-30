[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = square.e
  []

  [./extra_nodeset]
    type = ExtraNodesetGenerator
    input = fmg
    new_boundary = 'middle_node'
    coord = '0.5 0.5'
  []
[]

[Outputs]
  exodus = true
[]
