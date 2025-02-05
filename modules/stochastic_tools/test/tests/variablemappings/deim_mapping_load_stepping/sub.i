S = 10
D = 10
L = 5

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 10
    xmax = ${L}
    ymin = -1
    ymax = 1

  []

[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion_u]
    type = ADMatDiffusion
    variable = u
    diffusivity = D_u
  []
  [source_u]
    type = ADBodyForce
    variable = u
    value = 1.0
  []
[]

[Functions]
  [du]
    type = ParsedFunction
    expression = 'D * x^2 + 1'
    symbol_names = D
    symbol_values = ${D}
  []
[]

[Materials]
  [diffusivity_u]
    type = ADGenericFunctionMaterial
    prop_names = D_u
    prop_values = du
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = 'left top bottom'
    value = 0
  []
  [right_u]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = '${S} * t^2 * -(y^2-1)'
    preset = false
  []
[]

[Executioner]
  type = Transient
  dt = 0.25
  num_steps = 4
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               strumpack'

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
    execute_on = 'TIMESTEP_BEGIN'
  []
  [jacobian_storage]
    type = JacobianContainer
    tag_name = 'total'
    jac_indices_reporter_name = indices
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  console = true
[]

[Problem]
  extra_tag_vectors = 'total'
  extra_tag_matrices = 'total'
[]

[GlobalParams]
  extra_matrix_tags = 'total'
  extra_vector_tags = 'total'
[]
