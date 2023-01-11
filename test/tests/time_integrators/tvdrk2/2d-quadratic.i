[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 20
  ny = 20
  elem_type = QUAD9
[]

[Functions]
  [./ic]
    type = ParsedFunction
    expression = 0
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = 2*t*((x*x)+(y*y))-(4*t*t)
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*t*((x*x)+(y*y))
  [../]
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./ie]
    type = TimeDerivative
    variable = u
    implicit = true
  [../]

  [./diff]
    type = Diffusion
    variable = u
    implicit = false
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
    implicit = false
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = u
    function = ic
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
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

  [./TimeIntegrator]
    type = ExplicitTVDRK2
  [../]
  solve_type = 'LINEAR'

  start_time = 0.0
  num_steps = 10
  dt = 0.0001
  l_tol = 1e-8
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
