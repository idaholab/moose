[Mesh]
  [./eg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1'
    dy = '1'
    dz = '1'
    ix = '3'
    iy = '2'
    iz = '4'
    subdomain_id = '0'
  []
  [modifyNode]
    type = ModifyNodeGenerator
    input = eg
    node_id = '0 1 2'
    new_position = '-1 0 0 0
                     0 1 0 0
                     0 2 0 2'
  []
[]

[Outputs]
  exodus = true
[]
