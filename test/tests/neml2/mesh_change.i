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
  input = 'models/custom_model.i'
  verbose = true
  device = 'cpu'
  [A]
    model = 'model_A_non_ad'
    block = 'A'

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
  [B]
    model = 'model_B_non_ad'
    block = 'B'

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
  num_steps = 5
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
