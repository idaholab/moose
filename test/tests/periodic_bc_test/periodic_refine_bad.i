[Mesh]
  [./Generation]
    dim = 2
    nx = 8
    ny = 8
    nz = 0
    
    xmax = 40
    ymax = 40
    zmax = 0
    elem_type = QUAD4
  [../]

  uniform_refine = 4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff conv forcing dot'

  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 1e-5
  [../]

  [./conv]
    type = Convection
    variable = u
    x = -0.04
    y = 0
  [../]

  [./forcing]
    type = GaussContForcing
    variable = u
    x_center = 4.0
    y_center = 18.0
    x_spread = 1.0
    y_spread = 5.0
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
      type = DirichletBC
      variable = u
      primary = 3
      secondary = 1
      translation = '40 0 0'
    [../]

    [./y]
      type = DirichletBC
      variable = u
      primary = 0
      secondary = 2
      translation = '0 40 0'
    [../]
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Transient
  perf_log = true
  dt = 1
  num_steps = 80

  [./Adaptivity]
    refine_fraction = .25
    coarsen_fraction = .005
    max_h_level = 4
    error_estimator = KellyErrorEstimator
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]

