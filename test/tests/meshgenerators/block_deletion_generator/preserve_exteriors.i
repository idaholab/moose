[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8 # large enough to be interesting distributed
    ny = 8
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 4
  []
  [lower_d]
    type = LowerDBlockFromSidesetGenerator
    input = gmg
    new_block_name = 'external_boundary'
    sidesets = 'bottom top left right'
  []
  [del_lower_d_inner_block]
    type = BlockDeletionGenerator
    input = lower_d
    block = 0
    delete_exteriors = false
  []
[]
