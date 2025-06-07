[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[NEML2]
  input = 'models/custom_model.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

    moose_input_types = 'VARIABLE MATERIAL'
    moose_inputs = '     a        b'
    neml2_inputs = '     forces/A forces/B'

    moose_parameter_types = 'MATERIAL VARIABLE'
    moose_parameters = '     p1_mat   p2_var'
    neml2_parameters = '     p1       p2'

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
  [p2_var]
    initial_condition = 2
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
  [p1_mat]
    type = GenericConstantMaterial
    prop_names = 'p1_mat'
    prop_values = 3
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
