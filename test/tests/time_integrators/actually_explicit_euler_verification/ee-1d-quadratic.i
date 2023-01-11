[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 20
  elem_type = EDGE3
[]

[Functions]
  [./ic]
    type = ParsedFunction
    expression = 0
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = x*x-2*t
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*x*x
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
  [./ie]
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
    preset = false
    boundary = '0 1'
    function = exact_fn
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
  num_steps = 20
  dt = 0.00005

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
