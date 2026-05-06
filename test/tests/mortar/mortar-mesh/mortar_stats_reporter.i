[Mesh]
  [file]
    type = FileMeshGenerator
    file = '../gap-conductance-2d-non-conforming/nodal_normals_test_offset_nonmatching_gap.e'
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

[Variables]
  [T]
    block = '1 2'
  []
  [lambda]
    block = '10'
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  []
[]

[Constraints]
  [mortar]
    type = GapHeatConductanceTest
    primary_boundary = 2
    secondary_boundary = 1
    primary_subdomain = 20
    secondary_subdomain = 10
    variable = lambda
    secondary_variable = T
    debug_mesh = true
  []
[]

[Materials]
  [constant]
    type = ADGenericConstantMaterial
    prop_names = 'gap_conductance'
    prop_values = '.03'
    block = '1 2'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [mortar_stats]
    type = MortarSegmentMeshReporter
    execute_on = INITIAL
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
    execute_on = INITIAL
  []
[]
