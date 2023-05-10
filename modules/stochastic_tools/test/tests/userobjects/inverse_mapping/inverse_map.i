[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 10
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [v_pod]
  []
[]

[Kernels]
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
  [diffusivity_v]
    type = GenericConstantMaterial
    prop_names = D_v
    prop_values = 2.0
  []
[]

[BCs]
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
    value = 5
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
    parameters = '2 5'
    execute_on = TIMESTEP_END
  []
[]

[Surrogates]
  [polyreg]
    type = PolynomialRegressionSurrogate
    filename = "create_mapping_main_rom_polyreg.rd"
  []
[]

[VariableMappings]
  [pod]
    type = PODMapping
    filename = "create_mapping_main_mapping_pod_mapping.rd"
    num_modes_to_compute = 2
  []
[]

[Postprocessors]
  [error]
    type = ElementL2Difference
    variable = v
    other_variable = v_pod
    execute_on = FINAL
  []
[]

[Outputs]
  exodus = true
  execute_on = 'FINAL'
[]
