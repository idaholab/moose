[Mesh]
  file = 3d-non-conf.e

  [./MortarInterfaces]
    [./middle]
      master = 100
      slave = 101
      subdomain = 1000
    [../]
  [../]
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    value = x
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = 0
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  [../]

  [./lm]
    order = FIRST
    family = LAGRANGE
    block = middle
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[Constraints]
  [./ced]
    type = EqualValueConstraint
    variable = lm
    interface = middle
    master_variable = u
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '1 2 3 4 5 6'
    function = exact_fn
  [../]

  [./mortar]
    type = DiffusionFluxBC
    variable = u
    boundary = '100 101'
  [../]
[]

[Postprocessors]
  [./l2_error]
    type = ElementL2Error
    variable = u
    function = exact_fn
    block = '1 2'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-14
  l_tol = 1e-14
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
