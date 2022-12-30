[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2blk-conf.e
  []
  [secondary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    sidesets = '101'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    sidesets = '100'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
[]

[Functions]
  [./exact_sln]
    type = ParsedFunction
    expression= y
  [../]
  [./ffn]
    type = ParsedFunction
    expression= 0
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
    block = 'secondary_lower'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = BodyForce
    variable = u
    function = ffn
  [../]
[]

[Constraints]
  [./ced]
    type = EqualValueConstraint
    variable = lm
    secondary_variable = u
    primary_boundary = 100
    primary_subdomain = 10000
    secondary_boundary = 101
    secondary_subdomain = 10001
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '1 2 3 4'
    function = exact_sln
  [../]
[]

[Postprocessors]
  [./l2_error]
    type = ElementL2Error
    variable = u
    function = exact_sln
    block = '1 2'
    execute_on = 'initial timestep_end'
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
  exodus = true
[]
