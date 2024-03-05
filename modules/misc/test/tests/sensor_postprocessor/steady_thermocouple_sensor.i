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
    type = ADMatDiffusion
    variable = temperature
    diffusivity = thermal_conductivity
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
    prop_names = 'thermal_conductivity'
    prop_values = '18' # W/m*K @ 296K
  []
[]

[Postprocessors]
  [input_signal_pp]
    type = ElementAverageValue
    variable = temperature
  []
  [thermo_sensor_pp]
    type = ThermocoupleSensorPostprocessor
    input_signal = input_signal_pp
    uncertainty_std_dev_function = '0'
    noise_std_dev_function = '0'
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
