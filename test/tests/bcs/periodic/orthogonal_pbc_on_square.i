[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  nz = 0
  xmax = 10
  ymax = 10
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Functions]
  [./tr_x]
    type = ParsedFunction
    expression = 0
  [../]
  [./tr_y]
    type = ParsedFunction
    expression = x
  [../]
  [./itr_x]
    type = ParsedFunction
    expression = y
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
    y_center = 1
    x_spread = 0.25
    y_spread = 0.5
  [../]
  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  # active = ' '
  [./Periodic]
    [./x]
      primary = bottom
      secondary = left
      transform_func = 'tr_x tr_y'
      inv_transform_func = 'itr_x itr_y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 10
  solve_type = NEWTON
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

