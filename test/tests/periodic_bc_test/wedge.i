[Mesh]
  file = wedge.e
  uniform_refine = 1
[]

[Functions]
  active = 'tr_x tr_y'

  [./tr_x]
    type = ParsedFunction
    value = -x
  [../]

  [./tr_y]
    type = ParsedFunction
    value = y
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff forcing dot'

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
[]

[BCs]
  #active = ' '

  [./Periodic]
    [./x]
      primary = 1
      secondary = 2
      transform_func = 'tr_x tr_y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 6
[]

[Output]
  file_base = out_wedge
  interval = 1
  exodus = true
  perf_log = true
[]

