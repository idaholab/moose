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
  []
[]

[AuxVariables]
  [gap]
    block = '10'
  []
  [gap2]
    block = '10'
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
  []
  [gap2]
    type = GetMortarGapUOValue
    variable = gap2
    boundary = 1
    weighted_gap_uo = weighted_gap_uo
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
