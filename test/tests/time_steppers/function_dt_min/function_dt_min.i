[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax =  1
  ymin = -1
  ymax =  1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    value = t*t*(x*x+y*y)
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = 2*t*(x*x+y*y)-4*t*t
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[ICs]
  [./u_var]
    type = FunctionIC
    variable = u
    function = exact_fn
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient

  start_time = 0
  num_steps = 10
  [./TimeStepper]
    type = FunctionDT
    time_t  = '0   0.85 2'
    time_dt = '0.2 0.2  0.2'
    min_dt = 0.1
  [../]
[]

[Output]
  exodus = true
  output_initial = true
[]

