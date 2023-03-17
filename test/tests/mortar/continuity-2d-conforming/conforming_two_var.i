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

  [./lm_u]
    order = FIRST
    family = LAGRANGE
    block = 'secondary_lower'
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  [../]

  [./lm_v]
    order = FIRST
    family = LAGRANGE
    block = 'secondary_lower'
  [../]

[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = BodyForce
    variable = u
    function = ffn
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./coupled_u]
    type = CoupledForce
    variable = v
    v = u
  [../]
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Constraints]
  [./ced_u]
    type = EqualValueConstraint
    variable = lm_u
    secondary_variable = u
    primary_boundary = 100
    primary_subdomain = 10000
    secondary_boundary = 101
    secondary_subdomain = 10001
    absolute_value_vector_tags = 'ref'
  [../]
  [./ced_v]
    type = EqualValueConstraint
    variable = lm_v
    secondary_variable = v
    primary_boundary = 100
    primary_subdomain = 10000
    secondary_boundary = 101
    secondary_subdomain = 10001
    absolute_value_vector_tags = 'ref'
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
    execute_on = 'initial timestep_end'
  [../]
  [./l2_v]
    type = ElementL2Norm
    variable = v
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
  nl_rel_tol = 1e-12
  l_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
