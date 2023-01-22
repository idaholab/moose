[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 10
  elem_type = EDGE3
[]

[Functions]
  [./ic]
    type = ParsedFunction
    expression = 0
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = x*x-2*t+t*x*x
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*x*x
  [../]

  [./left_bc_fn]
    type = ParsedFunction
    expression = -t*2*x
  [../]
  [./right_bc_fn]
    type = ParsedFunction
    expression = t*2*x
  [../]
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = FunctionIC
      function = ic
    [../]
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

  [./abs]
    type = Reaction
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./left]
    type = FunctionNeumannBC
    variable = u
    boundary = '0'
    function = left_bc_fn
  [../]

  [./right]
    type = FunctionNeumannBC
    variable = u
    boundary = '1'
    function = right_bc_fn
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient

  l_tol = 1e-12
  start_time = 0.0
  num_steps = 10
  dt = 0.001

  [./TimeIntegrator]
    type = ActuallyExplicitEuler
  [../]
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    max_rows = 10
  [../]
[]
