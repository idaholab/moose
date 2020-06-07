[Mesh]
  second_order = true
  [./left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 3
    ny = 3
    elem_type = QUAD4
  [../]
  [./left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary_id = '0 1 2 3'
    new_boundary_name = 'lb_bottom lb_right lb_top lb_left'
  [../]
  [./left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  [../]
  [./right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    ymin = 0
    ymax = 1
    nx = 3
    ny = 3
    elem_type = QUAD4
  [../]
  [./right_block_id]
    type = SubdomainIDGenerator
    input = right_block
    subdomain_id = 2
  [../]
  [./combined]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  [../]
  [./block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block_id = '1 2'
    new_block_name = 'left_block right_block'
  [../]
  [right_right_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = block_rename
    new_boundary = rb_right
    block = right_block
    normal = '1 0 0'
  []
  [right_left_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_right_sideset
    new_boundary = rb_left
    block = right_block
    normal = '-1 0 0'
  []
  [right_top_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_left_sideset
    new_boundary = rb_top
    block = right_block
    normal = '0 1 0'
  []
  [right_bottom_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_top_sideset
    new_boundary = rb_bottom
    block = right_block
    normal = '0 -1 0'
  []
  [slave]
    input = right_bottom_sideset
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'lb_right'
    new_block_id = '10001'
    new_block_name = 'slave_lower'
  []
  [master]
    input = slave
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'rb_left'
    new_block_id = '10000'
    new_block_name = 'master_lower'
  []
[]

[Variables]
  [./T]
    block = 'left_block right_block'
    order = SECOND
  [../]
  [./lambda]
    block = 'slave_lower'
  [../]
[]

[BCs]
  [./neumann]
    type = FunctionGradientNeumannBC
    exact_solution = exact_soln_primal
    variable = T
    boundary = 'lb_bottom lb_top lb_left rb_bottom rb_right rb_top'
  [../]
[]

[Kernels]
  [./conduction]
    type = Diffusion
    variable = T
    block = 'left_block right_block'
  [../]
  [./sink]
    type = Reaction
    variable = T
    block = 'left_block right_block'
  [../]
  [./forcing_function]
    type = BodyForce
    variable = T
    function = forcing_function
    block = 'left_block right_block'
  [../]
[]

[Functions]
  [./forcing_function]
    type = ParsedFunction
    value = '-12*x^2 -24*y^2 - 2*x + x^4 + 2*y^4 + x*y^2'
  [../]
  [./exact_soln_primal]
    type = ParsedFunction
    value = 'x^4 + 2*y^4 + x*y^2'
  [../]
  [exact_soln_lambda]
    type = ParsedFunction
    value = '-y^2 - 15'
  []
[]

[Debug]
  show_var_residual_norms = 1
[]

[Constraints]
  [./mortar]
    type = GapHeatConductanceTest
    master_boundary = rb_left
    slave_boundary = lb_right
    master_subdomain = master_lower
    slave_subdomain = slave_lower
    slave_variable = T
    variable = lambda
    slave_gap_conductance = 1
    master_gap_conductance = 1
    slave_mms_function = 11
    master_mms_function = 17
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       superlu_dist'
[]

[Outputs]
  exodus = true
  csv = true
  [dofmap]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [L2lambda]
    type = ElementL2Error
    variable = lambda
    function = exact_soln_lambda
    execute_on = 'timestep_end'
    block = 'slave_lower'
  []
  [L2u]
    type = ElementL2Error
    variable = T
    function = exact_soln_primal
    execute_on = 'timestep_end'
    block = 'left_block right_block'
  []
  [h]
    type = AverageElementSize
    block = 'left_block right_block'
  []
[]
