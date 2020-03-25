[Mesh]
  [./gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []

  [./eng1]
    type = ExtraNodesetGenerator
    input = gen
    new_boundary = 'corner'
    coord = '0.5 1; 1 1; 1 0.5'
  []
  [./eng2]
    type = ExtraNodesetGenerator
    input = eng1
    new_boundary = 'single'
    coord = '0 0'
  []
[]

[Outputs]
  exodus = true
[]
