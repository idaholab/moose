[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [refine]
    type = RefineSidesetGenerator
    boundaries = 'left'
    refinement = '1'
    input = gmg
  []
  [lower_d_block]
    type = LowerDBlockFromSidesetGenerator
    input = refine
    new_block_id = 10
    sidesets = 'bottom right top left'
  []
[]
