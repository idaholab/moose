[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 6
  xmax = 6
[]

[Variables]
  [u]
  []
  [v]
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
  [diffusion_v]
    type = MatDiffusion
    variable = v
    diffusivity = D_v
  []
  [source_v]
    type = BodyForce
    variable = v
    value = 1.0
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
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 1
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
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
    tag_name = total
    execute_on = 'TIMESTEP_END'
  []
  [jacobian_storage]
    type = JacobianContainer
    tag_name = total
    jac_indices_reporter_name = indices
    execute_on = 'TIMESTEP_END'
  []
[]

[Problem]
  extra_tag_vectors = 'total'
  extra_tag_matrices = 'total'
[]

[GlobalParams]
  extra_matrix_tags = 'total'
  extra_vector_tags = 'total'
[]
