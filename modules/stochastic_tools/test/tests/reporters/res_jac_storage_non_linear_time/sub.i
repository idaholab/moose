[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 6
  xmax = 6
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion_u]
    type = MatDiffusion
    variable = u
    diffusivity = D_u
  []
  [source_u]
    type = BodyForce
    variable = u
    value = 1.0
  []
  [non_linear_u]
    type = ExponentialReaction
    variable = u
    mu1 = 7
    mu2 = 0.4
    extra_matrix_tags = 'nonlin_tag'
    extra_vector_tags = 'nonlin_tag'
  []
  [time_u]
    type = TimeDerivative
    variable = u
  []

[]

[Materials]
  [diffusivity_u]
    type = GenericConstantMaterial
    prop_names = D_u
    prop_values = 2.0
  []
  [diffusivity_v]
    type = GenericConstantMaterial
    prop_names = D_v
    prop_values = 4.0
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_rel_tol = 1e-16
  nl_abs_tol = 1e-12
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Reporters]
  [solution_storage]
    type = SolutionContainer
    execute_on = 'TIMESTEP_END'
  []
  [residual_storage]
    type = ResidualContainer
    tag_name = nonlin_tag
    execute_on = 'NONLINEAR'
  []
  [jacobian_storage]
    type = JacobianContainer
    tag_name = nonlin_tag
    jac_indices_reporter_name = indices
    execute_on = 'LINEAR TIMESTEP_END'
  []
[]

[Problem]
  extra_tag_vectors = 'nonlin_tag'
  extra_tag_matrices = 'nonlin_tag'
[]

