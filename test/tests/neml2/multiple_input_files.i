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

[NEML2]
  verbose = true
  device = 'cpu'
  [A]
    input = 'models/custom_model.i'
    model = 'model_A'
    block = 'A'

    # request derivatives (must be pairs of two)
    # derivative name follow moose convention, e.g., 'doutput/dinput'
    derivatives = 'product A'

    # output to exodus
    export_outputs = 'sum product dproduct/dA'
    export_output_targets = 'exodus; exodus; exodus'
  []
  [B]
    input = 'models/custom_model_2.i'
    model = 'model_B'
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
  []
[]

[Kernels]
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
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
