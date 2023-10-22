
[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 0.0
    ymin = -0.5
    ymax = 0.5
    nx = 2
    ny = 2
    elem_type = QUAD4
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []

  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.0
    xmax = 1.0
    ymin = .5
    ymax = 1.5
    nx = 3
    ny = 3
    elem_type = QUAD4
  []
  [right_block_sidesets]
    type = RenameBoundaryGenerator
    input = right_block
    old_boundary = '0 1 2 3'
    new_boundary = '20 21 22 23'
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block_sidesets
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
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff1]
    type = Diffusion
    variable = u
    block = 1
  [../]
  [./diff2]
    type = Diffusion
    variable = u
    block = 2
  [../]
[]

[Problem]
  kernel_coverage_check = false
  error_on_jacobian_nonzero_reallocation = true
[]

[BCs]
  [./fix_right]
    type = DirichletBC
    variable = u
    boundary = 21
    value = 0
  [../]
  [./Qx]
    type = DirichletBC
    variable = u
    boundary = 13
    value = 1
  [../]
[]

[Constraints]
  [mortar]
    type = PenaltyEqualValueConstraint
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1e3
    boundary_offset = '0 -1 0'
  []
[]

# [Executioner]
#  type = Steady
#  solve_type = 'PJFNK'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre boomeramg'
# []

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
#  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -mat_view'
#  petsc_options_value = 'lu superlu_dist ::ascii_matlab'
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]

