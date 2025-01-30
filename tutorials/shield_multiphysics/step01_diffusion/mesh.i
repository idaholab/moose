[Mesh]
  [bulk]
    type = GeneratedMeshGenerator # Can generate simple lines, rectangles and rectangular prisms
    dim = 3
    nx = 30
    ny = 30
    nz = 30
    xmax = 10 # m
    ymax = 13 # m
    zmax = 8 # m
  []

  [create_inner_water]
    type = ParsedSubdomainMeshGenerator
    input = bulk
    # Create each water wall
    combinatorial_geometry = '(x > 2 & x < 3 & y > 2 & y < 11 & z > 1 & z < 5) |
                  (x > 6.5 & x < 7.5 & y > 2 & y < 11 & z > 1 & z < 5) |
                  (x > 2 & x < 7.5 & y > 2 & y < 3 & z > 1 & z < 5) |
                  (x > 2 & x < 7.5 & y > 10 & y < 11 & z > 1 & z < 5)'
    block_id = 2
  []

  [hollow_concrete]
    type = ParsedElementDeletionGenerator
    input = create_inner_water
    expression = 'x > 3.5 & x < 6.5 & y > 3.5 & y < 9.5 & z > 1 & z < 5'
  []

  [rename_blocks]
    type = RenameBlockGenerator
    input = hollow_concrete
    old_block = '0 2'
    new_block = 'concrete water'
  []
[]
