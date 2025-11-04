[Mesh]
  parallel_type = distributed
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  [eng1]
    type = ExtraNodesetGenerator
    input = gen
    new_boundary = 'corner'
    coord = '0.99 0.99; -0.1 -0.1; 1.1 0'
    use_closest_node = true
  []
[]

[Outputs]
  exodus = true
[]
