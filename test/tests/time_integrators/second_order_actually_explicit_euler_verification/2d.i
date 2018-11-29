[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 3
  ny = 3
  elem_type = QUAD4
[]

[Functions]
  [./ic]
    type = ParsedFunction
    value = x+y
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = -4*pi^2*cos(2*pi*t)*(x+y)-2*pi*sin(2*pi*t)*(x+y)
  [../]

  [./exact_fn]
    type = ParsedFunction
    value = cos(2*pi*t)*(x+y)
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = FunctionIC
      function = ic
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
    boundary = '0 1 2 3'
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

  start_time = 0.0
  end_time = 5e-4
  dt = 5e-5
  l_tol = 1e-12

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
