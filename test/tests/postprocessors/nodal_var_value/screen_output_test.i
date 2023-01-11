[Mesh]
  file = square-2x2-nodeids.e
  # This test can only be run with renumering disabled, so the
  # NodalVariableValue postprocessor's node id is well-defined.
  allow_renumbering = false
[]

[Variables]
  active = 'u v'

  [./u]
    order = SECOND
    family = LAGRANGE
  [../]

  [./v]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Functions]
  active = 'force_fn exact_fn left_bc'

  [./force_fn]
    type = ParsedFunction
    expression = '1-x*x+2*t'
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = '(1-x*x)*t'
  [../]

  [./left_bc]
    type = ParsedFunction
    expression = t
  [../]
[]

[Kernels]
  active = '
    time_u diff_u ffn_u
    time_v diff_v'

  [./time_u]
    type = TimeDerivative
    variable = u
  [../]

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./ffn_u]
    type = BodyForce
    variable = u
    function = force_fn
  [../]

  [./time_v]
    type = TimeDerivative
    variable = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  active = 'all_u left_v right_v'

  [./all_u]
    type = FunctionDirichletBC
    variable = u
    boundary = '1'
    function = exact_fn
  [../]

  [./left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = '3'
    function = left_bc
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = '2'
    value = 0
  [../]
[]

[Postprocessors]
  active = 'l2 node1 node4'

  [./l2]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]

  [./node1]
    type = NodalVariableValue
    variable = u
    nodeid = 15
  [../]

  [./node4]
    type = NodalVariableValue
    variable = v
    nodeid = 10
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  dt = 0.1
  start_time = 0
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  [./console]
    type = Console
    max_rows = 2
  [../]
[]
