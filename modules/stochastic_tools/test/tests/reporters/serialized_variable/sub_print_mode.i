[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 10
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[AuxVariables]
  [u_pod]
  []
  [v_pod]
  []
[]

[Problem]
  solve = false
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
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[UserObjects]
  [im]
    type = InverseMapping
    mapping = pod
    surrogate = polyreg
    variable_to_fill = "v_pod"
    variable_to_reconstruct = "v"
    parameters = '2 2'
    execute_on = INITIAL
  []
[]

[Surrogates]
  [polyreg]
    type = PolynomialRegressionSurrogate
    filename = "main_2d_mc_rom_polyreg.rd"
  []
[]

[Mappings]
  [pod]
    type = PODMapping
    filename = "main_2d_mc_rd_pod_mapping.rd"
  []
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Reporters]
  [pod_coeffs]
    type = MappingReporter
    mapping = pod
    variables = "v"
  []
[]

[Outputs]
  exodus = true
  execute_on = 'INITIAL TIMESTEP_END'
[]
