[Mesh]
  file = timesequence_restart1_cp/0002_mesh.cpr
[]

[Problem]
  restart_file_base = timesequence_restart1_cp/0002
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

[Executioner]
  type = Transient
  end_time = 4.5
  [./TimeStepper]
    type = TimeSequenceStepper
    time_sequence  = '0   0.85 1.3 1.9 2 4 4.5'
  [../]
[]

[Outputs]
  exodus = true
[]
