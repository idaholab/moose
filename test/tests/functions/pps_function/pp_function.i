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
    type = BodyForce
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

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  exodus = true
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
    execute_on = linear
    boundary = 2
    type = SideAverageValue
  [../]
[]
