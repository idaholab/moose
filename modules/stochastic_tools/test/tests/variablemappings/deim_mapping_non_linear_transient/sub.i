S = 20
D = 20

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 250
    ny = 250
  []

[]

[Variables]
  [u]
  []
[]

[Kernels]
  [time]
    type = CoefTimeDerivative
    variable = u
  []
  [diffusion_u]
    type = ADMatDiffusion
    variable = u
    diffusivity = D_u
  []
  [non_linear_u]
    type = ExponentialReaction
    variable = u
    mu1 = ${S}
    mu2 = ${D}
  []
  [source_u]
    type = ADBodyForce
    variable = u
    function = '100*sin(2*pi*x)*sin(2*pi*y)'
  []
[]

[Materials]
  [diffusivity_u]
    type = ADGenericFunctionMaterial
    prop_names = D_u
    prop_values = 1
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = 'left top bottom right'
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 5
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               strumpack'
  residual_and_jacobian_together = true
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
    tag_name = 'total'
    execute_on = 'TIMESTEP_BEGIN  NONLINEAR_CONVERGENCE TIMESTEP_END'
  []
  [jacobian_storage]
    type = JacobianContainer
    tag_name = 'total'
    jac_indices_reporter_name = indices
    execute_on = ' NONLINEAR_CONVERGENCE TIMESTEP_END'
    execution_order_group = 1
  []
[]

[Outputs]
  console = true
  # exodus = true
  [perf]
    type = PerfGraphOutput
    level = 3
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
