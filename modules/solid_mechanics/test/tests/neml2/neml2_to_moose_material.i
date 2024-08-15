[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
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
    outputs = exodus
  []

  [neml2_sum]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    moose_material_property = neml2_sum
    neml2_variable = state/internal/sum
    outputs = exodus
  []

  [neml2_product]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    moose_material_property = neml2_product
    neml2_variable = state/internal/product
    outputs = exodus
  []

  [neml2_dproduct_da]
    type = NEML2ToRealMOOSEMaterialProperty
    execute_neml2_model_uo = model
    moose_material_property = neml2_dproduct_da
    # dproduct/dA
    neml2_variable = state/internal/product
    neml2_input_derivative = forces/A
    outputs = exodus
  []
[]

[NEML2]
  input = 'models/neml2_test_model.i'
  model = 'model'
  device = 'cpu'
  mode = PARSE_ONLY
  enable_AD = true
[]

[UserObjects]
  # correct dependencies are construction order independent
  [model]
    type = ExecuteNEML2Model
    model = model
    enable_AD = true
    gather_uos = 'gather_a gather_b'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []

  [gather_a]
    type = MOOSEVariableToNEML2
    moose_variable = a
    neml2_variable = forces/A
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [gather_b]
    type = MOOSERealMaterialPropertyToNEML2
    moose_material_property = b
    neml2_variable = forces/B
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
