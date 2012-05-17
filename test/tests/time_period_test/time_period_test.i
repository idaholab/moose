[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Functions]
  [./exact_p1]
    type = ParsedFunction
    value = t*((x*x)+(y*y))
  [../]
  [./ffn_p1]
    type = ParsedFunction
    value = (x*x+y*y)-4*t
  [../]
  
  [./exact_p2]
    type = ParsedFunction
    value = t*((x*x*x)+(y*y*y))
  [../]
  [./ffn_p2]
    type = ParsedFunction
    value = (x*x*x+y*y*y)-6*t*(x+y)
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
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
  
  [./ffn1]
    type = UserForcingFunction
    variable = u
    function = ffn_p1
    time_periods = p1
  [../]
  
  [./ffn2]
    type = UserForcingFunction
    variable = u
    function = ffn_p2
    time_periods = p2
  [../]
[]

[BCs]
  [./all1]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_p1
    time_periods = p1
  [../]
  [./all2]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_p2
    time_periods = p2
  [../]
[]

[Executioner]
  type = Transient
  
  start_time = 0
  dt = 0.1
  num_steps = 10

  time_periods       = 'p1 p2'
  time_period_starts = '0  0.45'
[]

[Output]
  output_initial = true
  exodus = true
[]
