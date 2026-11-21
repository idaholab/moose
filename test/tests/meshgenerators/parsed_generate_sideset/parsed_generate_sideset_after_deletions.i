[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 4
    ymax = 2
    zmax = 2
    nx = 4
    ny = 2
    nz = 2
  []

  [blocks]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '2 1 2'
  []

  [cut_1]
    type = ParsedElementDeletionGenerator
    input = blocks
    expression = 'x > 3'
  []

  # Needs the neighbors left by the first deletion to find the new exterior
  [cut_1_ss]
    type = ParsedGenerateSideset
    input = cut_1
    combinatorial_geometry = 'x > 2.9'
    new_sideset_name = cut_1_ss
    include_only_external_sides = true
  []

  [cut_2]
    type = ParsedElementDeletionGenerator
    input = cut_1_ss
    expression = 'x > 2 & y > 1'
  []

  [cut_2_ss]
    type = ParsedGenerateSideset
    input = cut_2
    combinatorial_geometry = 'x > 1.9 & y > 0.9'
    new_sideset_name = cut_2_ss
    include_only_external_sides = true
  []

  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = cut_2_ss
    primary_block = 1
    paired_block = 0
    new_boundary = interface
  []
[]

[Outputs]
  exodus = true
[]
