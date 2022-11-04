# Test implementation of sodium properties by comparison to analytical functions.

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [./temperature]
  [../]
[]

[AuxKernels]
  [./temperature_aux]
    type = FunctionAux
    variable = temperature
    function = '400 + 200 * t'
  [../]
[]

[Functions]
  [./k]
    type = ParsedFunction
    symbol_names = 'T'
    symbol_values = 'temperature'
    expression = '124.67 - 0.11381 * T + 5.5226e-5 * T^2 - 1.1842e-8 * T^3'
  [../]
  [./h]
    type = ParsedFunction
    symbol_names = 'T'
    symbol_values = 'temperature'
    expression = '1.0e3 * (-365.77 + 1.6582 * T - 4.2395e-4 * T^2 + 1.4847e-7 * T^3 + 2992.6 / T)'
  [../]
  [./cp]
    type = ParsedFunction
    symbol_names = 'T'
    symbol_values = 'temperature'
    expression = '1.0e3 * (1.6582 - 8.4790e-4 * T + 4.4541e-7 * T^2 - 2992.6 / T^2)'
  [../]
  [./rho]
    type = ParsedFunction
    symbol_names = 'T'
    symbol_values = 'temperature'
    expression = '219.0 + 275.32 * (1.0 - T / 2503.7) + 511.58 * (1.0 - T / 2503.7)^(0.5)'
  [../]
  [./drho_dT]
    type = ParsedFunction
    symbol_names = 'T'
    symbol_values = 'temperature'
    expression = '-(2.0 * 275.32 + 511.58 / (1.0 - T / 2503.7)^(0.5)) / 2.0 / 2503.7'
  [../]
  [./drho_dh]
    type = ParsedFunction
    symbol_names = 'drho_dT_exact cp_exact'
    symbol_values = 'drho_dT_exact cp_exact'
    expression = 'drho_dT_exact/cp_exact'
  [../]
[]

[FluidProperties/sodium]
  type = SodiumProperties
[]

[Materials]
  [./fp_mat]
    type = SodiumPropertiesMaterial
    temperature = temperature
    outputs = all
  [../]
[]


[Executioner]
  type = Transient

  num_steps = 10
[]

[Postprocessors]
  [./temperature]
    type = ElementAverageValue
    variable = temperature
    outputs = none
  [../]
  [./k_exact]
    type = FunctionValuePostprocessor
    function = k
    outputs = none
  [../]
  [./h_exact]
    type = FunctionValuePostprocessor
    function = h
    outputs = none
  [../]
  [./cp_exact]
    type = FunctionValuePostprocessor
    function = cp
    outputs = none
  [../]
  [./rho_exact]
    type = FunctionValuePostprocessor
    function = rho
    outputs = none
  [../]
  [./drho_dT_exact]
    type = FunctionValuePostprocessor
    function = drho_dT
    outputs = none
  [../]
  [./drho_dh_exact]
    type = FunctionValuePostprocessor
    function = drho_dh
    outputs = none
  [../]
  [./k_avg]
    type = ElementAverageValue
    variable = k
    outputs = none
  [../]
  [./h_avg]
    type = ElementAverageValue
    variable = h
    outputs = none
  [../]
  [./cp_avg]
    type = ElementAverageValue
    variable = cp
    outputs = none
  [../]
  [./t_from_h_avg]
    type = ElementAverageValue
    variable = temperature
    outputs = none
  [../]
  [./rho_avg]
    type = ElementAverageValue
    variable = rho
    outputs = none
  [../]
  [./drho_dT_avg]
    type = ElementAverageValue
    variable = drho_dT
    outputs = none
  [../]
  [./drho_dh_avg]
    type = ElementAverageValue
    variable = drho_dh
    outputs = none
  [../]
  [./k_diff]
    type = DifferencePostprocessor
    value1 = k_exact
    value2 = k_avg
  [../]
  [./h_diff]
    type = DifferencePostprocessor
    value1 = h_exact
    value2 = h_avg
  [../]
  [./cp_diff]
    type = DifferencePostprocessor
    value1 = cp_exact
    value2 = cp_avg
  [../]
  [./t_from_h_diff]
    type = DifferencePostprocessor
    value1 = temperature
    value2 = t_from_h_avg
  [../]
  [./rho_avg_diff]
    type = DifferencePostprocessor
    value1 = rho_exact
    value2 = rho_avg
  [../]
  [./drho_dT_avg_diff]
    type = DifferencePostprocessor
    value1 = drho_dT_exact
    value2 = drho_dT_avg
  [../]
  [./drho_dh_avg_diff]
    type = DifferencePostprocessor
    value1 = drho_dh_exact
    value2 = drho_dh_avg
  [../]
[]

[Outputs]
  csv = true
[]
