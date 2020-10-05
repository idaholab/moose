[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]

  [./extra_nodeset]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = 'middle_node'
    coord = '0.5 0.5'
  []
[]

[Outputs]
  exodus = true
[]
