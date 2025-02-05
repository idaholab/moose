S = 9
D = 5

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 25
    ny = 25
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

[]

[Problem]
  solve = false
[]

[VariableMappings]
  [rb_mapping]
    type = DEIMRBMapping
    filename = 'parallel_storage_main_mapping_rb_mapping.rd'
  []
[]

[UserObjects]
  [im]
    type = InverseRB
    mapping = rb_mapping
    execute_on = 'INITIAL TIMESTEP_END'
    relaxation_factor = 0.5
    max_iter = 10
  []
[]

[Outputs]
  exodus = true
[]
