# checking scale and translation, with ordering scale first, then translation second
[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax = 1
  nx = 3
  ymin = -1
  ymax = 1
  ny = 3
  zmin = -1
  zmax = 1
  nz = 3
[]

[UserObjects]
  [./solution_uo]
    type = SolutionUserObject
    mesh = cube_with_u_equals_x.e
    timestep = 1
    system_variables = u
    scale = '0.5 1 1'
    translation = '2 0 0'
    transformation_order = 'scale translation'
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
  file_base = solution_function_scale_transl
  exodus = true
[]
