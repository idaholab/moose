[Mesh]
  [fmg_left]
    type = FileMeshGenerator
    file = left.e
  []

  [fmg_center]
    type = FileMeshGenerator
    file = center.e
  []

  [rename_center]
    type = RenameBoundaryGenerator
    input = fmg_center
    old_boundary = 'right'
    new_boundary = 'bazinga'
  []

  [smg]
    type = StitchedMeshGenerator
    inputs = 'fmg_left rename_center'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'right left;'
    parallel_type = 'replicated'
    prevent_boundary_ids_overlap = false
  []
[]

[Outputs]
  exodus = true
[]
