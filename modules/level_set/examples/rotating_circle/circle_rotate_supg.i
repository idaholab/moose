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
  [./vel_x]
  [../]
  [./vel_y]
  [../]
[]

[AuxKernels]
  [./vel_x]
    type = FunctionAux
    function = 4*y
    variable = vel_x
    execute_on = initial
  [../]
  [./vel_y]
    type = FunctionAux
    function = -4*x
    variable = vel_y
    execute_on = initial
  [../]
[]

[Variables]
  [./phi]
  [../]
[]

[Functions]
  [./phi_exact]
    type = LevelSetOlssonBubble
    epsilon = 0.03
    center = '0 0.5 0'
    radius = 0.15
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
  [./advection_supg]
    type = LevelSetAdvectionSUPG
    velocity_x = vel_x
    velocity_y = vel_y
    variable = phi
  [../]
  [./time_supg]
    type = LevelSetTimeDerivativeSUPG
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

[Outputs]
  csv = true
  exodus = true
[]
