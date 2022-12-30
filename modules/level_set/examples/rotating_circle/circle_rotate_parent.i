[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 32
  ny = 32
  uniform_refine = 2
[]

[AuxVariables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
[]

[Variables]
  [./phi]
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = phi
    boundary = 'top bottom left right'
    value = 0
  [../]
[]

[Functions]
  [./phi_exact]
    type = LevelSetOlssonBubble
    epsilon = 0.03
    center = '0 0.5 0'
    radius = 0.15
  [../]
  [./velocity_func]
    type = ParsedVectorFunction
    expression_x = '4*y'
    expression_y = '-4*x'
  [../]
[]

[ICs]
  [./phi_ic]
    type = FunctionIC
    function = phi_exact
    variable = phi
  [../]
  [./vel_ic]
    type = VectorFunctionIC
    variable = velocity
    function = velocity_func
  []
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = phi
  [../]

  [./advection]
    type = LevelSetAdvection
    velocity = velocity
    variable = phi
  [../]
[]

[Postprocessors]
  [./area]
    type = LevelSetVolume
    threshold = 0.5
    variable = phi
    location = outside
    execute_on = 'initial timestep_end'
  [../]
  [./cfl]
    type = LevelSetCFLCondition
    velocity = velocity
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  start_time = 0
  end_time = 1.570796
  scheme = crank-nicolson
  petsc_options_iname = '-pc_type -pc_sub_type'
  petsc_options_value = 'asm      ilu'
  [./TimeStepper]
    type = PostprocessorDT
    postprocessor = cfl
    scale = 0.8
  [../]
[]

[MultiApps]
  [./reinit]
    type = LevelSetReinitializationMultiApp
    input_files = 'circle_rotate_sub.i'
    execute_on = 'timestep_end'
  [../]
[]

[Transfers]
  [./to_sub]
    type = MultiAppCopyTransfer
    source_variable = phi
    variable = phi
    to_multi_app = reinit
    execute_on = 'timestep_end'
  [../]
  [./to_sub_init]
    type = MultiAppCopyTransfer
    source_variable = phi
    variable = phi_0
    to_multi_app = reinit
    execute_on = 'timestep_end'
  [../]
  [./from_sub]
    type = MultiAppCopyTransfer
    source_variable = phi
    variable = phi
    from_multi_app = reinit
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  csv = true
  exodus = true
[]
