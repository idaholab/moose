# Test implementation of TemperaturePressureFunctionFluidProperties properties by comparison to analytical functions.

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [temperature]
  []
  [pressure]
  []
[]

[ICs]
  [temperature_aux]
    type = FunctionIC
    variable = temperature
    function = '400 + 200 * t'
  []
  [pressure_aux]
    type = FunctionIC
    variable = pressure
    function = '1e5 + 20000 * t'
  []
[]

[Functions]
  [k]
    type = ParsedFunction
    symbol_names = 'T p'
    symbol_values = 'temperature pressure'
    expression = '14 + 1e-2 * T + 1e-5 * p'
  []
  [h]
    type = ParsedFunction
    symbol_names = 'T p'
    symbol_values = 'temperature pressure'
    expression = '2e3 + T - 1e-3 * p'
  []
  [cp]
    type = ParsedFunction
    symbol_names = 'T p'
    symbol_values = 'temperature pressure'
    expression = '(2e3 + 1.2 * T - 1e-3 * p) / 200'
  []
  [rho]
    type = ParsedFunction
    symbol_names = 'T p'
    symbol_values = 'temperature pressure'
    expression = '1.5e3 + 1.3 * T - 15e-4 * p'
  []
  [mu]
    type = ParsedFunction
    symbol_names = 'T p'
    symbol_values = 'temperature pressure'
    expression = '1e-3 + 2e-6 * T - 3e-9 * p'
  []
[]

[FluidProperties]
  [fp]
    type = TemperaturePressureFunctionFluidProperties
    cv = 4000
    k = k
    rho = rho
    mu = mu
  []
[]

[Materials]
  [to_vars]
    type = FluidPropertiesMaterialPT
    fp = fp
    outputs = 'all'
    # output_properties = 'density'
    pressure = pressure
    temperature = temperature
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  # Postprocessors to get from the functions used as fluid properties
  [temperature]
    type = ElementAverageValue
    variable = temperature
    outputs = none
  []
  [pressure]
    type = ElementAverageValue
    variable = pressure
    outputs = none
  []
  [k_exact]
    type = FunctionValuePostprocessor
    function = k
    outputs = none
  []
  [h_exact]
    type = FunctionValuePostprocessor
    function = h
    outputs = none
  []
  [cp_exact]
    type = FunctionValuePostprocessor
    function = cp
    outputs = none
  []
  [rho_exact]
    type = FunctionValuePostprocessor
    function = rho
    outputs = none
  []

  # Postprocessors to get from the fluid property object
  # [k_avg]
  #   type = ElementAverageValue
  #   variable = k
  #   outputs = none
  # []
  # [h_avg]
  #   type = ElementAverageValue
  #   variable = h
  #   outputs = none
  # []
  # [cp_avg]
  #   type = ElementAverageValue
  #   variable = cp
  #   outputs = none
  # []
  # [rho_avg]
  #   type = ElementAverageValue
  #   variable = density
  #   outputs = none
  # []

  # Postprocessors to compare the two
  # [k_diff]
  #   type = DifferencePostprocessor
  #   value1 = k_exact
  #   value2 = k_avg
  # []
  # [h_diff]
  #   type = DifferencePostprocessor
  #   value1 = h_exact
  #   value2 = h_avg
  # []
  # [cp_diff]
  #   type = DifferencePostprocessor
  #   value1 = cp_exact
  #   value2 = cp_avg
  # []
  # [rho_avg_diff]
  #   type = DifferencePostprocessor
  #   value1 = rho_exact
  #   value2 = rho_avg
  # []
[]

[Outputs]
  csv = true
  # exodus = true
[]
