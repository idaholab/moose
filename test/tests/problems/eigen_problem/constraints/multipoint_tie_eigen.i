[Mesh]
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
  # The coincident mid-height interface node of each half, for the postprocessors. The node sets
  # are made before the halves are combined, because the coordinate names two nodes afterwards.
  [primary_mid]
    type = ExtraNodesetGenerator
    input = left_half
    coord = '0.375 0.5'
    new_boundary = primary_mid
  []
  [secondary_mid]
    type = ExtraNodesetGenerator
    input = right_half
    coord = '0.375 0.5'
    new_boundary = secondary_mid
  []
  [split]
    type = CombinerGenerator
    inputs = 'primary_mid secondary_mid'
  []
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
    type = Reaction
    variable = u
    rate = -1.0
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
  [tie]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_boundary = right_half_left
    primary_boundary = left_half_right
  []
[]

[Executioner]
  type = Eigenvalue
  eigen_problem_type = gen_non_hermitian
  which_eigen_pairs = smallest_magnitude
  n_eigen_pairs = 1
  solve_type = newton
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Postprocessors]
  [u_secondary]
    type = AverageNodalVariableValue
    variable = u
    boundary = secondary_mid
    outputs = none
  []
  [u_primary]
    type = AverageNodalVariableValue
    variable = u
    boundary = primary_mid
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
