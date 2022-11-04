[Mesh]
  file = wedge.e
[]

[Functions]
  active = 'tr_x tr_y'

  [./tr_x]
    type = ParsedFunction
    expression = -x
  [../]

  [./tr_y]
    type = ParsedFunction
    expression = y
  [../]
[]

[Variables]
  active = 'u temp'
#  active = 'temp'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff forcing dot dot_T diff_T'
#  active = 'dot_T diff_T'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = GaussContForcing
    variable = u
    x_center = -0.5
    y_center = 3.0
    x_spread = 0.2
    y_spread = 0.2
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]

  [./dot_T]
    type = TimeDerivative
    variable = temp
  [../]

  [./diff_T]
    type = Diffusion
    variable = temp
  [../]
[]

[BCs]
  #active = ' '

  [./Periodic]
    [./x]
      primary = 1
      secondary = 2
      transform_func = 'tr_x tr_y'
      inv_transform_func = 'tr_x tr_y'
      variable = u
    [../]
  [../]

  [./left_temp]
    type = DirichletBC
    value = 0
    boundary = 1
    variable = temp
  [../]

  [./right_temp]
    type = DirichletBC
    value = 1
    boundary = 2
    variable = temp
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
  file_base = out_wedge_sys
  exodus = true
[]

