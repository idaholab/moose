[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 16
  ny = 16
[]

[Variables]
  [./phi]
  [../]
[]

[AuxVariables]
  [./velocity]
    family = LAGRANGE_VEC
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

[Functions]
  [./phi_exact]
    type = LevelSetOlssonBubble
    epsilon = 0.05
    center = '0.5 0.5 0'
    radius = 0.15
  [../]
  [./velocity_func]
    type = ParsedVectorFunction
    expression_x = '3'
    expression_y = '3'
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
  [./cfl]
    type = LevelSetCFLCondition
    velocity = velocity
    execute_on = 'initial'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  start_time = 0
  end_time = 1
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
