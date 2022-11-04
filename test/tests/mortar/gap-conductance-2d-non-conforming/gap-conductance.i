[Mesh]
  [file]
    type = FileMeshGenerator
    file = nodal_normals_test_offset_nonmatching_gap.e
  []
  [./primary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    sidesets = '2'
    new_block_id = '20'
  [../]
  [./secondary]
    input = primary
    type = LowerDBlockFromSidesetGenerator
    sidesets = '1'
    new_block_id = '10'
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./T]
    block = '1 2'
  [../]
  [./lambda]
    block = '10'
  [../]
[]

[BCs]
  [./neumann]
    type = FunctionGradientNeumannBC
    exact_solution = exact_soln
    variable = T
    boundary = '3 4 5 6 7 8'
  [../]
[]

[Kernels]
  [./conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  [../]
  [./sink]
    type = Reaction
    variable = T
    block = '1 2'
  [../]
  [./forcing_function]
    type = BodyForce
    variable = T
    function = forcing_function
    block = '1 2'
  [../]
[]

[Functions]
  [./forcing_function]
    type = ParsedFunction
    expression = '-4 + x^2 + y^2'
  [../]
  [./exact_soln]
    type = ParsedFunction
    expression = 'x^2 + y^2'
  [../]
[]

[Debug]
  show_var_residual_norms = 1
[]

[Constraints]
  [./mortar]
    type = GapHeatConductanceTest
    primary_boundary = 2
    secondary_boundary = 1
    primary_subdomain = 20
    secondary_subdomain = 10
    variable = lambda
    secondary_variable = T
  [../]
[]

[Materials]
  [constant]
    type = ADGenericConstantMaterial
    prop_names = 'gap_conductance'
    prop_values = '.03'
    block = '1 2'
  []
  [./ssm]
    type = SpatialStatefulMaterial
    block = '1 2'
  [../]
[]


[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type -snes_linesearch_type'
  petsc_options_value = 'lu       basic'
[]

[Outputs]
  exodus = true
  [dofmap]
    type = DOFMap
    execute_on = 'initial'
  []
[]
