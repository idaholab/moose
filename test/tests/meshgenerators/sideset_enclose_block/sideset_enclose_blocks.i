[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    dy = '1 1 1 1'
    subdomain_id = '1 2 1  2
                    1 3 4  4
                    2 5 10 1
                    1 2 2  2'
  []

  [add_sides]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '3 4 5 10'
    paired_block = '1 2'
    new_boundary = 'interior'
    input = cmg
  []

  [enclosed]
    type = SidesetsEncloseBlocks
    block = '3 4 5 10'
    boundary = '1 4'
    input = add_sides
  []
[]
