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
    expression = t*((x*x)+(y*y))
  [../]
  [./ffn_p1]
    type = ParsedFunction
    expression = (x*x+y*y)-4*t
  [../]

  [./exact_p2]
    type = ParsedFunction
    expression = t*((x*x*x)+(y*y*y))
  [../]
  [./ffn_p2]
    type = ParsedFunction
    expression = (x*x*x+y*y*y)-6*t*(x+y)
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
    type = BodyForce
    variable = u
    function = ffn_p1
  [../]

  [./ffn2]
    type = BodyForce
    variable = u
    function = ffn_p2
  [../]
[]

[BCs]
  [./all1]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_p1
  [../]
  [./all2]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_p2
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 0.1
  num_steps = 10
[]

[Controls]
  [./first_period]
    type = TimePeriod
    start_time = 0.0
    end_time = 0.45
    enable_objects = '*/ffn1 */all1'
    disable_objects = '*/ffn2 */all2'
    execute_on = 'initial timestep_begin'
    set_sync_times = true
  [../]
[]

[Outputs]
  exodus = true
[]
