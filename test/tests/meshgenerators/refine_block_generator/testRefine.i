[Mesh]
  [./eg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '2 1'
    ix = '2 1'
    subdomain_id = '0 1'
  []
  [refine]
    type = RefineBlockGenerator
    input = eg
    block = '0'
    refinement = 0
  []
[]

[Outputs]
  exodus = true
[]
