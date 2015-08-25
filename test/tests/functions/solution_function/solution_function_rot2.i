# checking rotation of points by 45 deg about y axis in a SolutionUserObject
[Mesh]
  # this is chosen so when i rotate through 45deg i get a length of "1" along the x or y or z direction
  type = GeneratedMesh
  dim = 3
  xmin = -0.70710678
  xmax = 0.70710678
  nx = 3
  ymin = -0.70710678
  ymax = 0.70710678
  ny = 3
  zmin = -0.70710678
  zmax = 0.70710678
  nz = 3
[]

[UserObjects]
  [./solution_uo]
    type = SolutionUserObject
    mesh = cube_with_u_equals_x.e
    timestep = 1
    system_variables = u
    rotation0_vector = '0 1 0'
    rotation0_angle = 45
    transformation_order = rotation0
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
  file_base = solution_function_rot2
  exodus = true
[]
