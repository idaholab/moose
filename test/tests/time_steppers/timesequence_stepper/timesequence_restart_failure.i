[Mesh]
  file = timesequence_restart_failure1_cp/0002-mesh.cpa.gz
[]

[Problem]
  restart_file_base = timesequence_restart_failure1_cp/0002
  # There is an initial conditions overwriting the restart on the nonlinear variable u
  # As you can see in the gold file, this makes the initial step output be from the
  # initial condition
  allow_initial_conditions_with_restart = true
[]

[Functions]
  [exact_fn]
    type = ParsedFunction
    expression = t*t*(x*x+y*y)
  []

  [forcing_fn]
    type = ParsedFunction
    expression = 2*t*(x*x+y*y)-4*t*t
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = SECOND
  []
[]

[ICs]
  [u_var]
    type = FunctionIC
    variable = u
    function = exact_fn
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = u
  []

  [diff]
    type = Diffusion
    variable = u
  []

  [ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = exact_fn
  []
[]

[Executioner]
  type = Transient
  end_time = 4.0
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '0   0.85 1.2 1.3 2 4'
  []
[]

[Outputs]
  exodus = true
[]
