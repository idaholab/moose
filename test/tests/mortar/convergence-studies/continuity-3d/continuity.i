[Mesh]
  second_order = false
  [./left_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 3
    nz = 3
    xmin = 0
    xmax = 0.333333333
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    elem_type = TET4
  [../]
  [./left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = 'lb_bottom lb_back lb_right lb_front lb_left lb_top'
  [../]
  [./left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  [../]
  [./right_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 3
    nz = 3
    xmin = 0.333333333
    xmax = 1
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    elem_type = TET4
  [../]
  [./right_block_id]
    type = SubdomainIDGenerator
    input = right_block
    subdomain_id = 2
  [../]
  [right_block_change_boundary_id]
    type = RenameBoundaryGenerator
    input = right_block_id
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '100 101 102 103 104 105'
  []
  [./combined]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_change_boundary_id'
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
    normal = '0 0 1'
  []
  [right_bottom_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_top_sideset
    new_boundary = rb_bottom
    block = right_block
    normal = '0 0 -1'
  []
  [right_front_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_bottom_sideset
    new_boundary = rb_front
    block = right_block
    normal = '0 1 0'
  []
  [right_back_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_front_sideset
    new_boundary = rb_back
    block = right_block
    normal = '0 -1 0'
  []
  [secondary]
    input = right_back_sideset
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'lb_right'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'rb_left'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./T]
    block = '1 2'
    order = FIRST
  [../]
  [./lambda]
    block = 'secondary_lower'
    family = LAGRANGE
    order = FIRST
    use_dual = true
  [../]
[]

[BCs]
  [./neumann]
    type = FunctionGradientNeumannBC
    exact_solution = exact_soln_primal
    variable = T
    boundary = 'lb_back lb_front lb_left lb_top lb_bottom rb_right rb_top rb_bottom rb_front rb_back'
  [../]
[]

[Kernels]
  [./conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  [../]
  [./sink]
    type = Reaction
    variable = T
    block = '1 2'
  [../]
  [./forcing_function]
    type = BodyForce
    variable = T
    function = forcing_function
    block = '1 2'
  [../]
[]

[Functions]
 [./forcing_function]
 type = ParsedFunction
 value = ''
 [../]
 [./exact_soln_primal]
 type = ParsedFunction
 value = ''
 [../]
 [exact_soln_lambda]
 type = ParsedFunction
 value = ''
 []
[]

[Debug]
  show_var_residual_norms = 1
[]

[Constraints]
  [./mortar]
    type = EqualValueConstraint
    primary_boundary = 'rb_left'
    secondary_boundary = 'lb_right'
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = lambda
    secondary_variable = T
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
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       basic                 NONZERO               1e-15'
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
 block = 'secondary_lower'
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
