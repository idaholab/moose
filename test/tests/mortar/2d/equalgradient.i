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

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  [../]

  [./lmx]
    order = FIRST
    family = LAGRANGE
    block = middle
  [../]
  [./lmy]
    order = FIRST
    family = LAGRANGE
    block = middle
  [../]
[]

[ICs]
  [./block1]
    type = FunctionIC
    variable = u
    block = 1
    function = y
  [../]
  [./block2]
    type = FunctionIC
    variable = u
    block = 2
    function = y-0.5
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
[]

[Constraints]
  [./cedx]
    type = EqualGradientConstraint
    variable = lmx
    interface = middle
    master_variable = u
    component = 0
  [../]
  [./cedy]
    type = EqualGradientConstraint
    variable = lmy
    interface = middle
    master_variable = u
    component = 1
  [../]
[]

[BCs]
  [./all]
    type = DiffusionFluxBC
    variable = u
    boundary = '2 4 100 101'
  [../]
  [./boundary]
    type = DirichletBC
    boundary = 1
    variable = u
    value = 0.0
  [../]
  [./top]
    type = FunctionDirichletBC
    boundary = 3
    variable = u
    function = 0.5-t
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
  type = Transient
  nl_rel_tol = 1e-11
  l_tol = 1e-10
  l_max_its = 10
  dt = 0.05
  num_steps = 3
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
