[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[NEML2]
  input = 'models/neml2_test_model.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

    moose_input_types = 'VARIABLE MATERIAL'
    moose_inputs = '     a        b'
    neml2_inputs = '     forces/A forces/B'

    moose_output_types = 'MATERIAL           MATERIAL'
    moose_outputs = '     neml2_sum          neml2_product'
    neml2_outputs = '     state/internal/sum state/internal/product'

    moose_derivative_types = 'MATERIAL'
    moose_derivatives = 'neml2_dproduct_da'
    neml2_derivatives = 'state/internal/product forces/A'

    export_outputs = 'neml2_sum neml2_product neml2_dproduct_da'
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
    reaction_rate = neml2_sum
  []
  [reaction_2]
    type = MatReaction
    variable = u
    reaction_rate = neml2_product
  []
  [reaction_3]
    type = MatReaction
    variable = u
    reaction_rate = neml2_dproduct_da
  []
[]

[AuxVariables]
  [a]
  []
[]

[ICs]
  [a]
    type = FunctionIC
    variable = a
    function = 'x'
  []
[]

[Materials]
  [b]
    type = GenericFunctionMaterial
    prop_names = 'b'
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
