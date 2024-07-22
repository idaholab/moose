[Mesh]
  [./fmg_left]
    type = FileMeshGenerator
    file = left.e
  []

  [./fmg_center]
    type = FileMeshGenerator
    file = center.e
  []

  [./fmg_right]
    type = FileMeshGenerator
    file = right.e
  []

  [./smg]
    type = StitchedMeshGenerator
    inputs = 'fmg_left fmg_center fmg_right'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'right left;
                               right left'
    parallel_type = 'replicated'
    merge_boundaries_with_same_name = false
  []
[]

[Outputs]
  exodus = true
[]
