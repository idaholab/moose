[Mesh]
  file = trapezoid.e
  uniform_refine = 1
[]

# Polar to Cartesian
# R = sqrt(x^2 + y^2)
# x = R * cos(theta)
# y = R * sin(theta)
[Functions]
  [./tr_x]
    type = ParsedFunction
    value = sqrt(x^2+y^2)*cos(2*pi/3)
  [../]

  [./tr_y]
    type = ParsedFunction
    value = sqrt(x^2+y^2)*sin(2*pi/3)
  [../]

  [./itr_x]
    type = ParsedFunction
    value = sqrt(x^2+y^2)*cos(0)
  [../]

  [./itr_y]
    type = ParsedFunction
    value = sqrt(x^2+y^2)*sin(0)  # Always Zero!
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

  # A forcing term near the periodic boundary
  [./forcing]
    type = ExampleGaussContForcing
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
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
