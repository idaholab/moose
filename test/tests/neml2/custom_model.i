[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[NEML2]
  eager = true
  input = 'models/custom_model.i'
  # NEML2TestModel is hosted inside MOOSE as a Python module; importing it registers the type with
  # the embedded (cpp-eager) interpreter so the input file can reference it.
  load = 'models/test_models.py'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

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
