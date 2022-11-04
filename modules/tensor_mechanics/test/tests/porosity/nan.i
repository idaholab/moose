[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [volumetric]
    type = ParsedFunction
    expression = 't * sqrt(-1)'
    # expression = 0
    # expression =
  []
  [exact]
    type = ParsedFunction
    symbol_names = 'f'
    symbol_values = 'porosity_old'
    expression = '(1 - f) * 3e-3 + f'
  []
[]

[Materials]
  [porosity]
    type = PorosityFromStrain
    initial_porosity = 0
    negative_behavior = zero
    inelastic_strain = strain
    outputs = all
  []
  [strain]
    type = GenericFunctionRankTwoTensor
    tensor_name = strain
    tensor_functions = 'volumetric'
    outputs = all
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1e-3
[]

[Postprocessors]
  [porosity]
    type = ElementAverageValue
    variable = porosity
    execute_on = 'initial timestep_end'
  []
  [porosity_old]
    type = ElementAverageValue
    variable = porosity
    execute_on = 'initial timestep_begin'
    outputs = none
  []
  [exact]
    type = FunctionValuePostprocessor
    function = exact
  []
  [00]
    type = ElementAverageValue
    variable = strain_00
    execute_on = 'initial timestep_end'
  []
  [11]
    type = ElementAverageValue
    variable = strain_11
    execute_on = 'initial timestep_end'
  []
  [22]
    type = ElementAverageValue
    variable = strain_22
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
