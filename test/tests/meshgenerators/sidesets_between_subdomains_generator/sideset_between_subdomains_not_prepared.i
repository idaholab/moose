[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1'
    ix = '2 2'
    iy = '2'
    subdomain_id = '1 2'
  []
  [feg]
    type = FancyExtruderGenerator
    input = cmg
    direction = '0 0 1'
    heights = 1
    num_layers = 1
  []
  [sbsg]
    type = SideSetsBetweenSubdomainsGenerator
    input = feg
    new_boundary = 'interface'
    paired_block = 2
    primary_block = 1
  []
[]
