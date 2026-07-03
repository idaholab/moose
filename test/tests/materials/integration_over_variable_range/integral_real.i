[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 1
[]

[Problem]
  solve = false
[]

[Variables]
  [T]
    order = CONSTANT
    family = MONOMIAL

    [InitialCondition]
      type = FunctionIC
      function = '2'
    []
  []
[]

[Materials]
  [integrand_constant]
    type = GenericConstantMaterial
    prop_names  = 'constant'
    prop_values = '2'
  []
  [integrand_linear]
    type = ParsedMaterial
    property_name = 'linear'
    expression = '2 * T'
    coupled_variables = 'T'
  []
  [integrated]
    type = IntegrationOverVariableRangeMaterial
    # Properties
    input_prop_names = 'constant linear'
    output_prop_names = 'integrated_cte integrated_lin'

    # Reference
    integration_variable = 'T'
    prop_reference_values = '1 1'
    variable_reference_value = 0

    # expect outputs:
    # (reference) 1
    # T = 1 -> 1; 1
    # T = 2 -> 1 + int(2, ref, 2)       ref = 1 -> 3
    #          1 + int(2 * T, ref, 2)   ref = 1 -> 4
    #                                   ref = 0 -> 5

    # Grid
    precompute_property_grid = false # TODO: try both!
    integration_dv = 0.001
    variable_min_grid_value = 0
    variable_max_grid_value = 5

    # Output for verification
    outputs = 'exodus'
    output_properties = 'integrated_cte integrated_lin'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [ct_min]
    type = ElementExtremeMaterialProperty
    mat_prop = 'integrated_cte'
    value_type = 'min'
  []
  [ct_max]
    type = ElementExtremeMaterialProperty
    mat_prop = 'integrated_cte'
    value_type = 'max'
  []
  [lin_min]
    type = ElementExtremeMaterialProperty
    mat_prop = 'integrated_lin'
    value_type = 'min'
  []
  [lin_max]
    type = ElementExtremeMaterialProperty
    mat_prop = 'integrated_lin'
    value_type = 'max'
  []
[]
