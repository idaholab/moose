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
  [heat_conduction_time_derivative]
    type = ADTimeDerivative
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

[Problem]
  type = FEProblem
[]

[Postprocessors]
  [input_signal_pp]
    type = ElementAverageValue
    variable = temperature
  []
  [thermo_sensor_pp]
    type = ThermocoupleSensorPostprocessor
    input_signal = input_signal_pp
    drift_function = '1'
    delay_function = '0.1'
    efficiency_function = '1'
    signalToNoise_function = '1.0'
    noise_std_dev_function = '1'
    uncertainty_std_dev_function = '1'
  []
[]

[Executioner]
  type = Transient
  line_search = none
  dt = 0.1
  num_steps = 50
  nl_rel_tol = 1e-02
  nl_abs_tol = 1e-8
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
