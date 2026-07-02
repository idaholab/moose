[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [A]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    block_name = A
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [B]
    type = SubdomainBoundingBoxGenerator
    input = A
    block_id = 2
    block_name = B
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
[]

[MeshModifiers]
  [AB]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = u
    criterion_type = ABOVE
    threshold = 0.5
    subdomain_id = 2
    complement_subdomain_id = 1
    execute_on = TIMESTEP_BEGIN
  []
[]

[NEML2]
  eager = true
  input = 'models/custom_model.i'
  load = 'models/test_models.py'
  verbose = true
  device = 'cpu'
  [A]
    model = 'model_A_non_ad'
    block = 'A'

    # request derivatives (must be pairs of two)
    # derivative name follow moose convention, e.g., 'doutput/dinput'
    derivatives = 'product A'

    # output to exodus
    export_outputs = 'sum product dproduct/dA'
    export_output_targets = 'exodus; exodus; exodus'
  []
  [B]
    model = 'model_B_non_ad'
    block = 'B'

    # request derivatives (must be pairs of two)
    # derivative name follow moose convention, e.g., 'doutput/dinput'
    derivatives = 'product A'

    # output to exodus
    export_outputs = 'sum product dproduct/dA'
    export_output_targets = 'exodus; exodus; exodus'
  []
[]

[Variables]
  [u]
    [InitialCondition]
      type = FunctionIC
      function = 'x'
    []
  []
[]

[Kernels]
  [rate]
    type = TimeDerivative
    variable = u
  []
  [diffusion]
    type = Diffusion
    variable = u
  []
  [reaction_1]
    type = MatReaction
    variable = u
    reaction_rate = sum
  []
  [reaction_2]
    type = MatReaction
    variable = u
    reaction_rate = product
  []
  [reaction_3]
    type = MatReaction
    variable = u
    reaction_rate = dproduct/dA
  []
[]

[AuxVariables]
  [A]
  []
[]

[ICs]
  [A]
    type = FunctionIC
    variable = A
    function = 'x'
  []
[]

[Materials]
  [B]
    type = GenericFunctionMaterial
    prop_names = 'B'
    prop_values = 'y+t'
    outputs = 'exodus'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 5
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
