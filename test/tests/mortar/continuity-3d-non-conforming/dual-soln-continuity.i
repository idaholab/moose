[Mesh]
  second_order = false
  [./left_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
    xmin = -0.6
    xmax = -0.1
    ymin = 0.25
    ymax = 0.75
    zmin = 0.25
    zmax = 0.75
    elem_type = TET4
  [../]
  [./left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 3 4 5'
    new_boundary = 'lb_back lb_bottom lb_top lb_left lb_front'
  [../]
  [./left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  [../]
  [./right_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 9
    nz = 9
    xmin = 0
    xmax = 0.5
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    elem_type = HEX8
  [../]
  [./right_block_id]
    type = SubdomainIDGenerator
    input = right_block
    subdomain_id = 901
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
    old_block_id = '1 901'
    new_block_name = 'left_block right_block'
  [../]
  [left_right_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = block_rename
    new_boundary = lb_right
    block = left_block
    normal = '1 0 0'
  []
  [right_right_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = left_right_sideset
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
  [right_front_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_bottom_sideset
    new_boundary = rb_front
    block = right_block
    normal = '0 0 1'
  []
  [right_back_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_front_sideset
    new_boundary = rb_back
    block = right_block
    normal = '0 0 -1'
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
    block = '1 901'
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
    exact_solution = exact_soln
    variable = T
    boundary = 'lb_back lb_front lb_left lb_top lb_bottom rb_right rb_top rb_bottom rb_front rb_back'
  [../]
[]

[Kernels]
  [./conduction]
    type = Diffusion
    variable = T
    block = '1 901'
  [../]
  [./sink]
    type = Reaction
    variable = T
    block = '1 901'
  [../]
  [./forcing_function]
    type = BodyForce
    variable = T
    function = forcing_function
    block = '1 901'
  [../]
[]

[Functions]
  [./forcing_function]
    type = ParsedFunction
    value = '-8 + x^2 + y^2 + z^2'
  [../]
  [./exact_soln]
    type = ParsedFunction
    value = 'x^2 + y^2 + z^2'
  [../]
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
  [dofmap]
    type = DOFMap
    execute_on = 'initial'
  []
[]
