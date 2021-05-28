[Mesh]
  [msh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    dy = '1 1 1'
    subdomain_id = '0 0 0 0 1 2 2 1 1 1 1 1'
  []
  [split]
    input = msh
    type = BreakMeshByBlockGenerator
    split_interface = true
  []
[]
