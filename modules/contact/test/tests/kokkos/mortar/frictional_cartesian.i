# Frictional mortar contact of two blocks with a Cartesian Lagrange multiplier on the current
# configuration: the left block is pressed onto the right block and slides along it. KokkosDiffusion
# of each displacement component stands in for the mechanics. The mesh and the frictional contact
# objects are those of mortar_cartesian_lms/two_block_1st_order_constraint_lm_xy_friction.i, and the
# gold file is a link to that of frictional_cartesian_reference.i, its non-Kokkos version.
theta = 0
velocity = 0.1
refine = 3

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.35
    xmax = -0.05
    ymin = -1
    ymax = 0
    nx = 1
    ny = 3
    elem_type = QUAD4
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [left_block_sideset_names]
    type = RenameBoundaryGenerator
    input = left_block_sidesets
    old_boundary = '10 11 12 13'
    new_boundary = 'l_bottom l_right l_top l_left'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sideset_names
    subdomain_id = 1
  []
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.3
    ymin = -1
    ymax = 0
    nx = 1
    ny = 2
    elem_type = QUAD4
  []
  [right_block_sidesets]
    type = RenameBoundaryGenerator
    input = right_block
    old_boundary = '0 1 2 3'
    new_boundary = '20 21 22 23'
  []
  [right_block_sideset_names]
    type = RenameBoundaryGenerator
    input = right_block_sidesets
    old_boundary = '20 21 22 23'
    new_boundary = 'r_bottom r_right r_top r_left'
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block_sideset_names
    subdomain_id = 2
  []

  [combined_mesh]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  []

  [left_lower]
    type = LowerDBlockFromSidesetGenerator
    input = combined_mesh
    sidesets = '11'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [right_lower]
    type = LowerDBlockFromSidesetGenerator
    input = left_lower
    sidesets = '23'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []

  [rotate_mesh]
    type = TransformGenerator
    input = right_lower
    transform = ROTATE
    vector_value = '0 0 ${theta}'
  []

  uniform_refine = ${refine}
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
  [lm_x]
    block = 'secondary_lower'
    use_dual = true
  []
  [lm_y]
    block = 'secondary_lower'
    use_dual = true
  []
[]

[Kernels]
  [disp_x]
    type = KokkosDiffusion
    variable = disp_x
  []
  [disp_y]
    type = KokkosDiffusion
    variable = disp_y
  []
[]

[UserObjects]
  [kinematics_uo]
    type = KokkosMortarContactKinematicsUserObject
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    lm_variable = lm_x
    compute_slip = true
  []
[]

[Constraints]
  [friction]
    type = KokkosComputeCartesianLMMechanicalContact
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = lm_x
    lm_y = lm_y
    kinematics_uo = kinematics_uo
    c = 1e2
    c_t = 1e2
    mu = 0.4
  []
  [force]
    type = KokkosCartesianMortarMechanicalContact
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = lm_x
    lm_y = lm_y
  []
[]

[Functions]
  [horizontal_movement]
    type = KokkosParsedFunction
    expression = '${velocity} * t * cos(${theta}/180*pi)'
  []
  [vertical_movement]
    type = KokkosParsedFunction
    expression = '${velocity} * t * sin(${theta}/180*pi) + 0.05 * t'
  []
[]

[BCs]
  [push_left_x]
    type = KokkosFunctionDirichletBC
    variable = disp_x
    boundary = 13
    function = horizontal_movement
  []
  [push_left_y]
    type = KokkosFunctionDirichletBC
    variable = disp_y
    boundary = 13
    function = vertical_movement
  []
  [fix_right_x]
    type = KokkosDirichletBC
    variable = disp_x
    boundary = 21
    value = 0
  []
  [fix_right_y]
    type = KokkosDirichletBC
    variable = disp_y
    boundary = 21
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
  line_search = none
  dt = 0.1
  dtmin = 0.1
  end_time = 1.0
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
