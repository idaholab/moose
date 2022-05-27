[Mesh]
  [top_half]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 94 #750/8
    ny = 16 #125
    xmax = 30 #30
    ymin = 0
    ymax = 5 #5
    boundary_id_offset = 0
    boundary_name_prefix = top_half
  []
  [top_nonstitch]
    type = BoundingBoxNodeSetGenerator
    input = top_half
    new_boundary = top_nonstitch
    bottom_left = '0 0 0' #5
    top_right = '5 0 0' #30
    show_info = true
  []
  [top_stitch]
    type = BoundingBoxNodeSetGenerator
    input = top_nonstitch
    new_boundary = top_stitch
    bottom_left = '5 0 0' #5
    top_right = '30 0 0' #30
    show_info = true
  []
  [bottom_half]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 94 #0
    ny = 16 #5
    xmax = 30 #0
    ymin = -5
    ymax = 0
    boundary_id_offset = 7
    boundary_name_prefix = bottom_half
  []
  [bottom_nonstitch]
    type = BoundingBoxNodeSetGenerator
    input = bottom_half
    new_boundary = bottom_nonstitch
    bottom_left = '0 0 0'
    top_right = '5 0 0'
    show_info = true
  []
  [bottom_stitch]
    type = BoundingBoxNodeSetGenerator
    input = bottom_nonstitch
    new_boundary = bottom_stitch
    bottom_left = '5 0 0'
    top_right = '30 0 0'
    show_info = true
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'top_stitch bottom_stitch'
    stitch_boundaries_pairs = 'top_stitch bottom_stitch'
    show_info = true
  []
  construct_side_list_from_node_list = true
[]
[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]
[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]
[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]
