[Mesh]
  file = trapezoid.e
  uniform_refine = 1
[]

[Functions]
  [./tr_x]
    type = ParsedFunction
    expression = -x*cos(pi/3)
  [../]

  [./tr_y]
    type = ParsedFunction
    expression = x*sin(pi/3)
  [../]

  [./itr_x]
    type = ParsedFunction
    expression = -x/cos(pi/3)
  [../]

  [./itr_y]
    type = ParsedFunction
    expression = 0
  [../]
[]

[Variables]
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

  [./forcing]
    type = GaussContForcing
    variable = u
    x_center = 2
    y_center = -1
    x_spread = 0.25
    y_spread = 0.5
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./Periodic]
    [./x]
      primary = 1
      secondary = 4
      transform_func = 'tr_x tr_y'
      inv_transform_func = 'itr_x itr_y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 6
  solve_type = NEWTON
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_trapezoid
  exodus = true
[]
