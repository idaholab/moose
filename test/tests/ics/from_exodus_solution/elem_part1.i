# We run a simple problem (5 time steps and save off the solution)
# In part2, we load the solution and solve a steady problem. The test check, that the initial state in part 2 is the same as the last state from part1

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 20
  ny = 20
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = t*((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = -4+(x*x+y*y)
  [../]
[]

[AuxVariables]
  [./e]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./ak]
    type = FunctionAux
    variable = e
    function = exact_fn
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'ie diff ffn'

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

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  dt = 0.2
  start_time = 0
  num_steps = 5
[]

[Outputs]
  exodus = true
[]
