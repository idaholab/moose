[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = invalid_orientation.msh
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = file
    sidesets = secondary
    new_block_id = 100
    new_block_name = secondary_lower
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = primary
    new_block_id = 200
    new_block_name = primary_lower
  []
[]

[Variables]
  [disp_x]
    block = 'secondary_volume primary_volume'
  []
  [disp_y]
    block = 'secondary_volume primary_volume'
  []
  [normal_lm]
    block = secondary_lower
  []
[]

[Kernels]
  [disp_x]
    type = Diffusion
    variable = disp_x
    block = 'secondary_volume primary_volume'
  []
  [disp_y]
    type = Diffusion
    variable = disp_y
    block = 'secondary_volume primary_volume'
  []
[]

[UserObjects]
  [weighted_gap_uo]
    type = LMWeightedGapUserObject
    primary_boundary = primary
    secondary_boundary = secondary
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    lm_variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
  []
[]

[Constraints]
  [weighted_gap_lm]
    type = ComputeWeightedGapLMMechanicalContact
    primary_boundary = primary
    secondary_boundary = secondary
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    c = 1
    weighted_gap_uo = weighted_gap_uo
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
