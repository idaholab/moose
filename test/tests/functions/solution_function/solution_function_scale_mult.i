# checking scale_multiplier
[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  nx = 3
  ymin = -1
  ymax = 1
  ny = 3
[]

[UserObjects]
  [./solution_uo]
    type = SolutionUserObject
    mesh = square_with_u_equals_x.e
    timestep = 1
    system_variables = u
    scale_multiplier = '2 2 0'
    transformation_order = scale_multiplier
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./u_init]
    type = FunctionIC
    variable = u
    function = solution_fcn
  [../]
[]

[Functions]
  [./solution_fcn]
    type = SolutionFunction
    from_variable = u
    solution = solution_uo
  [../]
[]

[Kernels]
  [./diff]
    type = TimeDerivative
    variable = u
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  l_max_its = 800
  nl_rel_tol = 1e-10
  num_steps = 1
  end_time = 1
  dt = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = solution_function_scale_mult
  exodus = true
[]
