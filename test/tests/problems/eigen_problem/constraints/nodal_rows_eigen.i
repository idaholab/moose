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
    formulation = rows
    primary = '39'
    secondary_node_ids = '10'
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
  n_eigen_pairs = 1
  n_basis_vectors = 20
  solve_type = newton
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Postprocessors]
  [u_secondary]
    type = NodalVariableValue
    variable = u
    nodeid = 10
    outputs = none
  []
  [u_primary]
    type = NodalVariableValue
    variable = u
    nodeid = 39
    outputs = none
  []
  [tie_error]
    type = DifferencePostprocessor
    value1 = u_secondary
    value2 = u_primary
  []
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
