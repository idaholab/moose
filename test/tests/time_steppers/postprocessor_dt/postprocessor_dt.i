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
    expression = t*t*(x*x+y*y)
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = 2*t*(x*x+y*y)-4*t*t
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
    type = BodyForce
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

[Postprocessors]
  [./l2_error]
    type = ElementL2Error
    variable = u
    function = exact_fn
    execute_on = 'initial timestep_end'
  [../]

  # Just use some postprocessor that gives values good enough for time stepping ;-)
  [./dt]
    type = ElementAverageValue
    variable = u
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'crank-nicolson'

  start_time = 1.0
  num_steps = 2
  [./TimeStepper]
    type = PostprocessorDT
    postprocessor = dt
  [../]
[]

[Outputs]
  exodus = true
[]
