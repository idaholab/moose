[Mesh]
  file = 2blk-conf.e

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
    value = y
  [../]
  [./ffn]
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

  [./v]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  [../]

  [./lmv]
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
    function = ffn
  [../]
  [./diffv]
    type = Diffusion
    variable = v
  [../]
  [./coupleu]
    type = OptionallyCoupledForce
    variable = v
    v = u
  [../]
[]

[Constraints]
  [./ced]
    type = EqualValueConstraint
    variable = lm
    interface = middle
    master_variable = u
  [../]
  [./vc]
    type = EqualValueConstraint
    variable = lmv
    interface = middle
    master_variable = v
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '1 2 3 4'
    function = exact_sln
  [../]
  [./allv]
    type = DirichletBC
    variable = v
    boundary = '1 2 3 4'
    value = 0
  [../]
[]

[Postprocessors]
  [./l2_error]
    type = ElementL2Error
    variable = u
    function = exact_sln
    block = '1 2'
  [../]
  [./l2_v]
    type = ElementL2Norm
    variable = v
    block = '1 2'
  [../]
[]

[Preconditioning]
  [./fmp]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-11
  l_tol = 1e-10
[]

[Outputs]
  output_initial = true
  exodus = true
  print_perf_log = true
[]
