[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    # A 1 by 0.7 rectangle, not a square: the square's second and third modes are a
    # degenerate pair, and Krylov-Schur on the singular-M mortar problem drops one member
    # of that pair when its subspace is small or its arithmetic differs (n_basis_vectors of
    # 12 or 16 locally, 20 on one CIVET build). The rectangle's first four modes are distinct.
    ymax = 0.7
    nx = 8
    ny = 8
    elem_type = QUAD4
  []
  [left_half]
    type = RenameBlockGenerator
    input = square
    old_block = '0'
    new_block = 'left_half'
  []
  [right_half]
    type = SubdomainBoundingBoxGenerator
    input = left_half
    block_id = 2
    block_name = 'right_half'
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = right_half
    split_interface = true
  []
  [secondary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = split
    sidesets = 'left_half_right_half'
    new_block_id = 10001
    new_block_name = 'secondary_lower'
  []
  [primary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = secondary_lower
    sidesets = 'right_half_left_half'
    new_block_id = 10000
    new_block_name = 'primary_lower'
  []
  [interface_ends]
    type = ExtraNodesetGenerator
    input = primary_lower
    new_boundary = 'interface_ends'
    nodes = '4 76'
  []
  allow_renumbering = false
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    block = 'left_half right_half'
  []
  [lm]
    order = FIRST
    family = LAGRANGE
    block = 'secondary_lower'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [rhs]
    type = CoefReaction
    variable = u
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    variable = u
    boundary = 'left right bottom top'
    value = 0
  []
  [eigen]
    type = EigenDirichletBC
    variable = u
    boundary = 'left right bottom top'
  []
  [lm_ends]
    type = DirichletBC
    variable = lm
    boundary = 'interface_ends'
    value = 0
  []
  [lm_ends_eigen]
    type = EigenDirichletBC
    variable = lm
    boundary = 'interface_ends'
  []
[]

[Constraints]
  [tie]
    type = EqualValueConstraint
    variable = lm
    secondary_variable = u
    primary_boundary = 'right_half_left_half'
    primary_subdomain = 'primary_lower'
    secondary_boundary = 'left_half_right_half'
    secondary_subdomain = 'secondary_lower'
  []
[]

[Problem]
  use_hash_table_matrix_assembly = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Eigenvalue
  eigen_problem_type = gen_non_hermitian
  which_eigen_pairs = smallest_magnitude
  n_eigen_pairs = 4
  n_basis_vectors = 20
  solve_type = krylovschur
  petsc_options_iname = '-st_type -eps_target -st_pc_type -st_pc_factor_mat_solver_type'
  petsc_options_value = 'sinvert 0 lu mumps'
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
