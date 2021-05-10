[Mesh]
  [msh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    dy = '1'
    ix = '1 1 1 1'
    iy = 1
    subdomain_id = '0 1 2 3'
  []
  [split]
    input = msh
    type = BreakMeshByBlockGenerator
    block_pairs = '0 1;
                   2 3'
    split_interface = true
  []
[]
