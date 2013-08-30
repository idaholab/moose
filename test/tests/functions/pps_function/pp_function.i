[Mesh]
  file = square.e
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./function_force]
    function = pp_func
    variable = u
    type = UserForcingFunction
  [../]
[]

[BCs]
  active = 'left right'
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]

[Functions]
  [./pp_func]
    pp = right_value
    type = PostprocessorFunction
  [../]
[]

[Postprocessors]
  [./right_value]
    variable = u
    execute_on = residual
    boundary = 2
    type = SideAverageValue
  [../]
[]

