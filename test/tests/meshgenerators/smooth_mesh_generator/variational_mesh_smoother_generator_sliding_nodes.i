[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    bias_x = 1.4
    bias_y = 1.4
    elem_type = TRI3
  []

  [add_block]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x > 0.35 & y > 0.35 & y < 0.7'
    block_id = 1
  []

  [smooth]
    type = SmoothMeshGenerator
    input = add_block
    algorithm = variational
  []
[]
