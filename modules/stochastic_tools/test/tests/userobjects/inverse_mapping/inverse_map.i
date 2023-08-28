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
  [v_aux]
  []
  [v_aux_pod]
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

[AuxKernels]
  [func_aux]
    type = FunctionAux
    variable = v_aux
    function = v_aux_func
  []
[]

[Functions]
  [v_aux_func]
    type = ParsedFunction
    expression = 'S * x + D'
    symbol_names = 'S D'
    symbol_values = '2 5'
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
    surrogate = "polyreg_v polyreg_v_aux"
    variable_to_fill = "v_pod v_aux_pod"
    variable_to_reconstruct = "v v_aux"
    parameters = '2 5'
    execute_on = TIMESTEP_END
  []
[]

[Surrogates]
  [polyreg_v]
    type = PolynomialRegressionSurrogate
    filename = "create_mapping_main_rom_polyreg_v.rd"
  []
  [polyreg_v_aux]
    type = PolynomialRegressionSurrogate
    filename = "create_mapping_main_rom_polyreg_v_aux.rd"
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
  [error_v]
    type = ElementL2Difference
    variable = v
    other_variable = v_pod
    execute_on = FINAL
    outputs = csv_errors
  []
  [error_v_aux]
    type = ElementL2Difference
    variable = v_aux
    other_variable = v_aux_pod
    execute_on = FINAL
    outputs = csv_errors
  []
[]

[Outputs]
  exodus = true
  execute_on = 'FINAL'
  [csv_errors]
    type = CSV
  []
[]
