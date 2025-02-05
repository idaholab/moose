S = 10
D = 10
L = 5
P = 5

[Mesh]
  type = GeneratedMesh
  dim = 2
  ny = 500
  nx = 500
  xmax = ${L}
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
    type = ADBodyForce
    variable = u
    function = '1 + ${P} * cos(2*pi*x)^2 * cos(2*pi*y)^2'
  []
[]

[Functions]
  [du]
    type = ParsedFunction
    expression = '1 + D * sin(2*pi*x)^2 * sin(2*pi*y)^2 + ${S}'
    symbol_names = D
    symbol_values = ${D}
  []
[]

[Materials]
  [diffusivity_u]
    type = GenericFunctionMaterial
    prop_names = D_u
    prop_values = du
  []
[]

[BCs]
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
    preset = false
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg 2000'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-18
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

# [Dampers]
#   [slow]
#     type = ConstantDamper
#     damping = 0.99
#   []
# []

[Reporters]
  [solution_storage]
    type = SolutionContainer
    execute_on = 'TIMESTEP_END'
  []
  [residual_storage]
    type = ResidualContainer
    tag_name = total
    execute_on = 'TIMESTEP_BEGIN'
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

# [Outputs]
#   exodus = true
# []
