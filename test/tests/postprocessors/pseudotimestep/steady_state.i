[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 100
[]

[Variables]
  [u]
    initial_condition = 1.0
  []
[]

[Kernels]
  [diffusion]
    type = ADMatDiffusion
    variable = u
    diffusivity = 1
  []
  [reaction]
    type = ADMatReaction
    variable = u
    reaction_rate = L
  []
  [time_deriv]
    type = ADTimeDerivative
    variable = u
  []
[]

[Materials]
  [parsed]
    type = ADParsedMaterial
    expression = '1-u'
    coupled_variables = 'u'
    property_name = 'L'
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    value = 0
    boundary = 'left'
  []
  [right]
    type = ADDirichletBC
    variable = u
    value = 1
    boundary = 'right'
  []
[]

[Postprocessors]
  [pseudotimestep]
    type =PseudoTimestep
    method='SER'
    iterations_window=10
    initial_dt=20
    alpha=3
    max_dt=50
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-9
  end_time = 50

  [TimeStepper]
    type = PostprocessorDT
    postprocessor = pseudotimestep
  []

[]

[Outputs]
  print_linear_residuals = false
  csv=true
  execute_on = 'final'
[]
