[Mesh]
  file = 3d-non-conf-2nd.e
[]

[MortarInterfaces]
  [./middle]
    master = 100
    slave = 101
    subdomain = 1000
  [../]
[]

[Functions]
  [./exact_sln]
    type = ParsedFunction
    value = x*x+y*y+z*z
  [../]
  
  [./forcing_fn]
    type = ParsedFunction
    value = -6
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
    function = exact_sln
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
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-13
[]

[Output]
  interval = 1
  exodus = true
  perf_log = true
[]
