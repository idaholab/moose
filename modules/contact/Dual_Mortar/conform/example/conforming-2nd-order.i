refine = 1

[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2blk-conf-2nd.e
  []
  [slave]
    input = file
    type = LowerDBlockFromSidesetGenerator
    sidesets = '101'
    new_block_id = '10001'
    new_block_name = 'slave_lower'
  []
  [master]
    input = slave
    type = LowerDBlockFromSidesetGenerator
    sidesets = '100'
    new_block_id = '10000'
    new_block_name = 'master_lower'
  []
  uniform_refine =  ${refine}
[]

[Problem]
  kernel_coverage_check = false
[]

[Functions]
  [./exact_sln]
    type = ParsedFunction
    # value = x*x+y*y
    value = sin(2*pi*x)*sin(2*pi*y)
  [../]
  [./ffn]
    type = ParsedFunction
    # value = -4
    value = 8*pi*pi*sin(2*pi*x)*sin(2*pi*y)
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
    block = slave_lower
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
    slave_variable = u
    master_boundary = 100
    master_subdomain = 10000
    slave_boundary = 101
    slave_subdomain = 10001
    dual_mortar = true
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
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'

  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO   1e-15'

  # petsc_options_iname = '-pc_type -ksp_view_mat'
 	# petsc_options_value = 'svd ascii:matrix.m:ascii_matlab'

  nl_rel_tol = 1e-12
  l_tol = 1e-12
[]

[Outputs]
  [./exodus]
    type = Exodus
  [../]
  [dof_map]
    type = DOFMap
    execute_on = 'initial'
  []
[]
