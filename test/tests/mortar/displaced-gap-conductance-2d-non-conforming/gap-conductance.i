[Mesh]
  displacements = 'disp_x disp_y'
  [file]
    type = FileMeshGenerator
    file = nodal_normals_test_offset_nonmatching_gap.e
    # block 1: left
    # block 2: right
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

[AuxVariables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
[]

[AuxKernels]
  [function_x]
    type = FunctionAux
    function = '.05 * t'
    variable = 'disp_x'
    block = '2'
  []
  [function_y]
    type = FunctionAux
    function = '.05 * t'
    variable = 'disp_y'
    block = '2'
  []
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
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[BCs]
  [./left]
    type = ADDirichletBC
    variable = T
    boundary = '5'
    value = 0
  [../]
  [./right]
    type = ADDirichletBC
    variable = T
    boundary = '8'
    value = 1
  [../]
[]

[Kernels]
  [./conduction]
    type = ADDiffusion
    variable = T
    block = '1 2'
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
    use_displaced_mesh = true
    correct_edge_dropping = true
  [../]
[]

[Materials]
  [constant]
    type = ADGenericConstantMaterial
    prop_names = 'gap_conductance'
    prop_values = '.03'
    block = '1 2'
    use_displaced_mesh = true
  []
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  num_steps = 5
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
