[Mesh]
  [./smg]
    type = TestSubgenerators
    input_files = 'left.e center.e right.e'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'right left;
                               right left'
  []
[]

[Outputs]
  exodus = true
[]
