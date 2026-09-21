[Mesh]
  # A 3x8 and a 5x8 piece of the unit square that meet at x = 3/8 with duplicated
  # interface nodes. Combining them keeps every generated id: the left piece holds
  # nodes 0-35 and the right piece 36-89, both numbered x fastest, so the coincident
  # interface pairs are (36 + 6 j, 3 + 4 j) for j = 0..8. The cut is off centre on
  # purpose: two equal halves would each carry the same fundamental eigenvalue, so
  # the untied problem would be degenerate and the tie could go missing unnoticed.
  # BreakMeshByBlockGenerator gives the same geometry but numbers its duplicate
  # nodes in unordered_map order, which differs between standard libraries, so the
  # pairs below could not be hard-coded on it.
  [left_half]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.375
    ymin = 0
    ymax = 1
    nx = 3
    ny = 8
    elem_type = QUAD4
    subdomain_ids = 1
    boundary_name_prefix = left_half
  []
  [right_half]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.375
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 5
    ny = 8
    elem_type = QUAD4
    subdomain_ids = 2
    boundary_name_prefix = right_half
    boundary_id_offset = 10
  []
  [split]
    type = CombinerGenerator
    inputs = 'left_half right_half'
  []
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
    boundary = 'left_half_left left_half_bottom left_half_top right_half_right right_half_bottom right_half_top'
    value = 0
  []
  [eigen]
    type = EigenDirichletBC
    variable = u
    boundary = 'left_half_left left_half_bottom left_half_top right_half_right right_half_bottom right_half_top'
  []
[]

[Constraints]
  [tie_0]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 36
    primary_nodes = '3'
    weights = '1'
  []
  [tie_1]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 42
    primary_nodes = '7'
    weights = '1'
  []
  [tie_2]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 48
    primary_nodes = '11'
    weights = '1'
  []
  [tie_3]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 54
    primary_nodes = '15'
    weights = '1'
  []
  [tie_4]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 60
    primary_nodes = '19'
    weights = '1'
  []
  [tie_5]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 66
    primary_nodes = '23'
    weights = '1'
  []
  [tie_6]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 72
    primary_nodes = '27'
    weights = '1'
  []
  [tie_7]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 78
    primary_nodes = '31'
    weights = '1'
  []
  [tie_8]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_node = 84
    primary_nodes = '35'
    weights = '1'
  []
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
    nodeid = 60
    outputs = none
  []
  [u_primary]
    type = NodalVariableValue
    variable = u
    nodeid = 19
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
