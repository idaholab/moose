[Mesh]
  [file]
    type = FileMeshGenerator
    file = nodal_normals_test_offset_nonmatching_gap.e
  []
  [primary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    sidesets = '2'
    new_block_id = '20'
  []
  [secondary]
    input = primary
    type = LowerDBlockFromSidesetGenerator
    sidesets = '1'
    new_block_id = '10'
  []
  displacements = 'disp_x disp_y'
[]

[Problem]
  solve = false
[]

[UserObjects]
  [weighted_gap_uo]
    type = TestWeightedGapUserObject
    primary_boundary = 2
    secondary_boundary = 1
    primary_subdomain = 20
    secondary_subdomain = 10
    weighted_gap_aux_var = "gap2"
    execute_on = 'linear nonlinear timestep_end'
  []
[]

[AuxVariables]
  [gap]
    block = '10'
  []
  [gap2]
    block = '10'
  []
  [disp_x]
    block = 'left right'
  []
  [disp_y]
    block = 'left right'
  []
[]

[ICs]
  [disp_x]
    block = 'left'
    type = ConstantIC
    value = '-1e-2'
    variable = disp_x
  []
[]

[AuxKernels]
  [gap]
    type = WeightedGapAux
    variable = gap
    primary_boundary = 2
    secondary_boundary = 1
    primary_subdomain = 20
    secondary_subdomain = 10
    use_displaced_mesh = true
  []
  [gap2]
    type = GetMortarGapUOValue
    variable = gap2
    boundary = 1
    weighted_gap_uo = weighted_gap_uo
    execute_on = 'linear nonlinear timestep_end'
  []
[]

[Postprocessors]
  [gap]
    type = ElementAverageValue
    block = 10
    variable = gap
    execute_on = 'timestep_end'
    force_postaux = true
  []
  [gap2]
    type = ElementAverageValue
    block = 10
    variable = gap2
    execute_on = 'timestep_end'
    force_postaux = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
