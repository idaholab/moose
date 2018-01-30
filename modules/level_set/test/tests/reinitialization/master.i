[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 8
  ny = 8
  uniform_refine = 3 #1/64
[]

[AuxVariables]
  [./vel_x]
    initial_condition = 1
  [../]
  [./vel_y]
    initial_condition = 1
  [../]
[]

[Variables]
  [./phi]
  [../]
[]

[Functions]
  [./phi_exact]
    type = LevelSetOlssonBubble
    epsilon = 0.05
    center = '0.5 0.5 0'
    radius = 0.15
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      variable = phi
      auto_direction = 'x y'
    [../]
  [../]
[]

[ICs]
  [./phi_ic]
    type = FunctionIC
    function = phi_exact
    variable = phi
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = phi
  [../]

  [./advection]
    type = LevelSetAdvection
    velocity_x = vel_x
    velocity_y = vel_y
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
    velocity_x = vel_x
    velocity_y = vel_y
    execute_on = 'initial'
  [../]

[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  start_time = 0
  end_time = 1
  nl_rel_tol = 1e-12
  scheme = crank-nicolson
  petsc_options_iname = '-pc_type -pc_sub_type'
  petsc_options_value = 'asm      ilu'
  [./TimeStepper]
    type = PostprocessorDT
    postprocessor = cfl
    scale = 1
  [../]

[]

[MultiApps]
  [./reinit]
    type = LevelSetReinitializationMultiApp
    input_files = 'reinit.i'
    execute_on = 'timestep_end'
  [../]
[]

[Transfers]
  [./to_sub]
    type = MultiAppCopyTransfer
    variable = phi
    source_variable = phi
    direction = to_multiapp
    multi_app = reinit
    execute_on = 'timestep_end'
  [../]

  [./to_sub_init]
    type = MultiAppCopyTransfer
    variable = phi_0
    source_variable = phi
    direction = to_multiapp
    multi_app = reinit
    execute_on = 'timestep_end'
  [../]

  [./from_sub]
    type = MultiAppCopyTransfer
    variable = phi
    source_variable = phi
    direction = from_multiapp
    multi_app = reinit
    execute_on = timestep_end
  [../]
[]

[Outputs]
  exodus = true
  csv = true
[]
