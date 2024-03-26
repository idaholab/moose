# Test implementation of TemperaturePressureFunctionFluidProperties properties by comparison to analytical functions.

cv = 4000
T_initial = 400
p_initial = 1e5

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [temperature]
    initial_condition = ${T_initial}
  []
  [pressure]
    initial_condition = 1e5
  []
[]

[Functions]
  # This demonstrates how to define fluid properties that are functions
  # of the LOCAL value of the (p,T) variables
  # x for temperature
  # y for pressure
  [k]
    type = ParsedFunction
    expression = '14 + 1e-2 * x + 1e-5 * y'
  []
  [rho]
    type = ParsedFunction
    expression = '1.5e3 + 0.13 * x - 1.5e-4 * y'
  []
  [mu]
    type = ParsedFunction
    expression = '1e-3 + 2e-6 * x - 3e-9 * y'
  []
[]

[FluidProperties]
  [fp]
    type = TemperaturePressureFunctionFluidProperties
    cv = ${cv}
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
    output_properties = 'density k cp cv viscosity e h'
    pressure = pressure
    temperature = temperature

    compute_entropy = false
    compute_sound_speed = false
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k_exact]
    type = FunctionValuePostprocessor
    function = k
    outputs = none
    point = '${T_initial} ${p_initial} 0'
  []
  [rho_exact]
    type = FunctionValuePostprocessor
    function = rho
    outputs = none
    point = '${T_initial} ${p_initial} 0'
  []
  [mu_exact]
    type = FunctionValuePostprocessor
    function = mu
    outputs = none
    point = '${T_initial} ${p_initial} 0'
  []
  [e_exact]
    type = Receiver
    default = '${fparse cv * T_initial}'
    outputs = none
  []
  [cv_exact]
    type = Receiver
    default = '${fparse cv}'
    outputs = none
  []

  # Postprocessors to get from the fluid property object
  [k_avg]
    type = ElementAverageValue
    variable = k
    outputs = none
  []
  [rho_avg]
    type = ElementAverageValue
    variable = density
    outputs = none
  []
  [mu_avg]
    type = ElementAverageValue
    variable = viscosity
    outputs = none
  []
  [cv_avg]
    type = ElementAverageValue
    variable = cv
    outputs = none
  []
  [e_avg]
    type = ElementAverageValue
    variable = e
    outputs = none
  []

  # We output these directly, cant compare to anything analytical though
  [cp_avg]
    type = ElementAverageValue
    variable = cp
  []
  [h_avg]
    type = ElementAverageValue
    variable = h
  []

  # Postprocessors to compare the two
  [k_diff]
    type = DifferencePostprocessor
    value1 = k_exact
    value2 = k_avg
  []
  [mu_diff]
    type = DifferencePostprocessor
    value1 = mu_exact
    value2 = mu_avg
  []
  [rho_diff]
    type = DifferencePostprocessor
    value1 = rho_exact
    value2 = rho_avg
  []
  [e_diff]
    type = DifferencePostprocessor
    value1 = e_exact
    value2 = e_avg
  []
  [cv_diff]
    type = DifferencePostprocessor
    value1 = cv_exact
    value2 = cv_avg
  []
[]

[Outputs]
  # Note that diffs wont be settled until timestep 2 because of order of execution
  csv = true
[]
