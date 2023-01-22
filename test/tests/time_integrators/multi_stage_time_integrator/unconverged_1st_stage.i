# This test is designed to check that a time step solve should stop if *any*
# time integrator solve stage fails, not just the *last* stage. If a time
# integrator does not check convergence per stage, then a time step proceeds
# past intermediate stages without checking nonlinear convergence. This test
# is designed to check that the 2nd stage is never even entered by making it
# impossible for the first stage to converge.

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 5
[]

[Functions]
  [./ic]
    type = ParsedFunction
    expression = 0
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = x
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*x
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./body]
    type = BodyForce
    variable = u
    function = forcing_fn
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
  [./bcs]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1'
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient

  [./TimeIntegrator]
    type = LStableDirk2
  [../]
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = NEWTON
  nl_max_its = 0
[]
