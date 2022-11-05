[Mesh]
  file = wedge.e
  uniform_refine = 1
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

[AuxVariables]
  [two_u]
  []
[]

[AuxKernels]
  [two_u]
    type = ParsedAux
    variable = two_u
    coupled_variables = 'u'
    expression = '2*u'
  []
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
      inv_transform_func = 'tr_x tr_y'
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
  [png]
    type = PNGOutput
    resolution = 25
    color = RWB
    variable = 'two_u'
  []
[]
