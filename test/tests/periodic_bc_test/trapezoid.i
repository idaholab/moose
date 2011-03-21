[Mesh]
  file = trapezoid.e
  uniform_refine = 1
[]

[Functions]
  active = '
    tr_x tr_y tr_z
    itr_x itr_y itr_z'
    
  [./tr_x]
    type = ParsedFunction
    value = -x*cos(pi/3)
  [../]

  [./tr_y]
    type = ParsedFunction
    value = x*sin(pi/3)
  [../]

  [./tr_z]
    type = ParsedFunction
    value = 0
  [../]
  
  [./itr_x]
    type = ParsedFunction
    value = -x/cos(pi/3)
  [../]

  [./itr_y]
    type = ParsedFunction
    value = 0
  [../]

  [./itr_z]
    type = ParsedFunction
    value = 0
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
  #active = ' '

  [./Periodic]
    [./x]
      primary = 1
      secondary = 4
      transform_func = 'tr_x tr_y tr_z'
      inv_transform_func = 'itr_x itr_y itr_z'
    [../]
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Transient
  perf_log = true
  dt = 0.5
  num_steps = 6
[]

[Output]
  file_base = out_trapezoid
  interval = 1
  exodus = true
[]

