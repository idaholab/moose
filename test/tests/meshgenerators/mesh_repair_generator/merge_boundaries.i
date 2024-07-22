[Mesh]
  [gmg_left]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [gmg_left_renamed]
    type = RenameBoundaryGenerator
    input = gmg_left
    old_boundary = 'bottom right top left'
    new_boundary = 'side1 side2 side3 side4'
  []
  [gmg_right]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = 1
    xmax = 2
  []
  [gmg_right_renamed]
    type = RenameBoundaryGenerator
    input = gmg_right
    old_boundary = 'bottom right top left'
    new_boundary = 'side2 side3 side4 side1'
  []
  [smg]
    type = StitchedMeshGenerator
    inputs = 'gmg_left_renamed gmg_right_renamed'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'side2 side1;'
    merge_boundaries_with_same_name = false
  []
  [merge]
    type = MeshRepairGenerator
    input = 'smg'
    merge_boundary_ids_with_same_name = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [mesh]
    type = MeshInfo
    outputs = json
    items = 'sidesets sideset_elems'
  []
[]

[Outputs]
  [json]
    type = JSON
    execute_system_information_on = NONE
  []
[]
