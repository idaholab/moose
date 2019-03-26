[Mesh]
  file = 2blk-conf.e
[]

[MeshModifiers]
  [slave]
    type = LowerDBlockFromSideset
    sidesets = '101'
    new_block_id = '10001'
    new_block_name = 'slave_lower'
  []
  [master]
    type = LowerDBlockFromSideset
    sidesets = '100'
    new_block_id = '10000'
    new_block_name = 'master_lower'
  []
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

  [./lm_u]
    order = FIRST
    family = LAGRANGE
    block = 'slave_lower'
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  [../]

  [./lm_v]
    order = FIRST
    family = LAGRANGE
    block = 'slave_lower'
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

[Constraints]
  [./ced_u]
    type = EqualValueConstraint
    variable = lm_u
    master_variable = u
    master_boundary_id = 100
    master_subdomain_id = 10000
    slave_boundary_id = 101
    slave_subdomain_id = 10001
  [../]
  [./ced_v]
    type = EqualValueConstraint
    variable = lm_v
    master_variable = v
    master_boundary_id = 100
    master_subdomain_id = 10000
    slave_boundary_id = 101
    slave_subdomain_id = 10001
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
