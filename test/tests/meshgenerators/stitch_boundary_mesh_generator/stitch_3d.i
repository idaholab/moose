[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 4
    ny = 4
    nz = 4
    dim = 3
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 1'
    input = gen
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 1'
    input = block1
  []
  [breakmesh]
    input = block2
    type = BreakMeshByBlockGenerator
    block_pairs = '1 2'
    split_interface = true
    add_interface_on_two_sides = true
  []
  [block1_block2_top]
    type = SideSetsFromBoundingBoxGenerator
    input = breakmesh
    included_boundaries = 'Block1_Block2'
    boundary_new = '103'
    bottom_left = '0 0.5 0'
    top_right = '1 1 1'
  []
  [block1_block2_bottom]
    type = SideSetsFromBoundingBoxGenerator
    input = block1_block2_top
    included_boundaries = 'Block1_Block2'
    boundary_new = '102'
    bottom_left = '0 0.5 0'
    top_right = '1 1 1'
    location = OUTSIDE
  []

  [block2_block1_top]
    type = SideSetsFromBoundingBoxGenerator
    input = block1_block2_bottom
    included_boundaries = 'Block2_Block1'
    boundary_new = '101'
    bottom_left = '0 0.5 0'
    top_right = '1 1 1'
  []
  [block2_block1_bottom]
    type = SideSetsFromBoundingBoxGenerator
    input = block2_block1_top
    included_boundaries = 'Block2_Block1'
    boundary_new = '100'
    bottom_left = '0 0.5 0'
    top_right = '1 1 1'
    location = OUTSIDE
  []
  [stitch]
    type = StitchBoundaryMeshGenerator
    input = block2_block1_bottom
    clear_stitched_boundary_ids = false
    stitch_boundaries_pair = '101 103'
  []
[]

[Outputs]
  exodus = true
[]
