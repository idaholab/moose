[Mesh]
  type = GeneratedMesh
  dim = 1

  nx = 10

  xmin = 0.0
  xmax = 1.0

[]

#still need BC for Energy, IC's for both.
[Variables]
  active = 'Time'

  [./Time]
    order =  FIRST
    family = LAGRANGE
    initial_condition = 0.0
  [../]
[]

[Functions]
  active = 'func'

  [./func]
    type = ParsedFunction
    expression = 2.0*t
  [../]
[]

[Kernels]
  active = 't_time func_time'

  [./t_time]
    type = TimeDerivative
    variable = Time
  [../]

  [./func_time]
    type = BodyForce
    variable = Time
    function = func
  [../]
[]

[BCs]
  active = 'Top_Temperature'

  [./Top_Temperature]
    type = NeumannBC
    variable = Time
    boundary = 'left right'
  [../]

[]

[Executioner]
  type = Transient
  #scheme = 'BDF2'
  #scheme = 'crank-nicolson'
  start_time = 0
  num_steps = 4
  dt = 1000000000

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

  steady_state_tolerance = .00000000000000001
  steady_state_detection = true
  nl_abs_tol = 1e-15
  petsc_options = '-snes_converged_reason'
  abort_on_solve_fail = true
[]

[Outputs]
  file_base = out
  exodus = true
[]
