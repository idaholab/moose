[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 200
  elem_type = EDGE2
[]

[Functions]
  [./ic]
    type = ParsedFunction
    value = 0
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = -4*pi^2*sin(2*pi*t)*x+2*pi*cos(2*pi*t)*x
  [../]

  [./exact_fn]
    type = ParsedFunction
    value = sin(2*pi*t)*x
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = FunctionIC
      function = ic
      function_dot = 2*pi*x
    [../]
  [../]
[]

[Kernels]
  [./u_dotdot]
    type = SecondTimeDerivative
    variable = u
  [../]

  [./u_dot]
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
    boundary = '0 1'
    function = exact_fn
    extra_matrix_tags = 'secondtime'
    implicit = false
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
  num_steps = 10
  dt = 5e-5
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
