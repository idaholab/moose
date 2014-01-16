[Mesh]
  file = non-conf-coarse-2nd.e

  [./MortarInterfaces]
    [./middle]
      master = 100
      slave = 101
      subdomain = 1000
    [../]
  [../]
[]

[Functions]
  [./exact_sln]
    type = ParsedFunction
    value = x*x+y*y
  [../]
  [./ffn]
    type = ParsedFunction
    value = -4
  [../]
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE
    block = '1 2'
  [../]
  
  [./lm]
    order = SECOND
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
    function = ffn
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
    boundary = '1 2 3 4'
    function = exact_sln
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
    function = exact_sln
    block = '1 2'
  [../]
[]

[Preconditioning]
  [./fmp]
    type = FDP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-14
#  l_tol = 1e-14
[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
