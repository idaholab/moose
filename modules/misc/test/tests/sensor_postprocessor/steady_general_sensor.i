[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  coord_type = RZ
  rz_coord_axis = X
[]

[Variables]
  [temperature]
    initial_condition = 600 # Start at room temperature
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[BCs]
  [inlet_temperature]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 600 # (K)
  []
  [outlet_temperature]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 602 # (K)
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '18 0.466 80' # W/m*K, J/kg-K, kg/m^3 @ 296K
  []
[]

[Postprocessors]
  [input_signal_pp]
    type = ElementAverageValue
    variable = temperature
  []
  [general_sensor_pp]
    type = GeneralSensorPostprocessor
    input_signal = input_signal_pp
    noise_std_dev_function = '5'
    delay_function = '0.3'
    drift_function = '200'
    efficiency_function = '0.8'
    signalToNoise_function = '5'
    uncertainty_std_dev_function = '5'
    R_function = '1'
    proportional_weight = 1
    integral_weight = 1
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-02
  nl_abs_tol = 1e-8
[]

[Outputs]
  csv = true
[]
