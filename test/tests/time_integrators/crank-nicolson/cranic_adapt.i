[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    # dudt = 3*t^2*(x^2 + y^2)
    expression = sin(pi*x)*sin(pi*y)+2*t*pi*pi*sin(pi*x)*sin(pi*y)
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*sin(pi*x)*sin(pi*y)
  [../]
[]

[Kernels]
  active = 'diff ie ffn'

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

  # Use the block format instead of the scheme parameter
  [./TimeIntegrator]
    type = CrankNicolson
  [../]

  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 5
  dt = 0.1

  [./Adaptivity]
    refine_fraction = 0.2
    coarsen_fraction = 0.3
    max_h_level = 4
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
