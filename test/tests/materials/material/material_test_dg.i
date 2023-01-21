[Mesh]
  file = sq-2blk.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = MONOMIAL

    [./InitialCondition]
      type = ConstantIC
      value = 1
    [../]
  [../]
[]

[Functions]
  active = 'forcing_fn exact_fn'

  [./forcing_fn]
    type = ParsedFunction
    expression = (x*x*x)-6.0*x
  [../]

  [./exact_fn]
    type = ParsedGradFunction

    value = (x*x*x)
    grad_x = 3*x*x
    grad_y = 0
  [../]
[]

[Kernels]
  active = 'diff abs forcing'

  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = matp
  [../]

  [./abs]
    type = Reaction
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[DGKernels]
  active = 'dgdiff'

  [./dgdiff]
    type = DGDiffusion
    variable = u
    sigma = 6
    epsilon = -1.0
    diff = matp
  [../]
[]

[BCs]
  active = 'all'

  [./all]
    type = DGMDDBC
    variable = u
    boundary = '1 2 3 4'
    function = exact_fn
    prop_name = matp
    sigma = 6
    epsilon = -1.0
  [../]
[]

[Materials]
  active = 'mat_1 mat_2'

  [./mat_1]
    type = MTMaterial
    block = 1
    value = 1
  [../]

  [./mat_2]
    type = MTMaterial
    block = 2
    value = 2
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out_dg
  exodus = true
[]
