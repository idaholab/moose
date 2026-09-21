[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 8
  ny = 8
  elem_type = QUAD4
  allow_renumbering = false
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
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
[]

[Constraints]
  [tie]
    type = LinearNodalConstraint
    variable = u
    formulation = penalty
    primary = '39'
    secondary_node_ids = '41'
    weights = '1'
    penalty = 1e3
  []
[]

[Problem]
  use_hash_table_matrix_assembly = true
[]

[Executioner]
  type = Eigenvalue
  eigen_problem_type = gen_non_hermitian
  which_eigen_pairs = smallest_magnitude
  n_eigen_pairs = 3
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
