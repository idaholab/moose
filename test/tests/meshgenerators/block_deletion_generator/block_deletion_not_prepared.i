[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4 2 3'
    dy = '1 2'
    ix = '10 10 10'
    iy = '8 8'
    subdomain_id = '1 2 3
                    2 2 2'
  []
  [feg]
    type = AdvancedExtruderGenerator
    input = cmg
    direction = '0 0 1'
    heights = 1
    num_layers = 1
  []
  [bdg]
    type = BlockDeletionGenerator
    input = feg
    block = '1 3'
    new_boundary = 'new_external'
  []
[]
