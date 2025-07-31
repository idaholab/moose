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
    initial_condition = 293 # Start at room temperature
  []
[]

[Functions]
  [delay_func]
    type = PiecewiseLinear
    x = '0    0.5   1'
    y = '0.005 0.01   0.2'
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
    value = 293 # (K)
  []
  [outlet_temperature]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 650 # (K)
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1 0.466 80' # W/m*K, J/kg-K, kg/m^3 @ 296K
  []
[]

[Problem]
  type = FEProblem
[]

[Postprocessors]
  [input_signal_pp]
    type = ElementAverageValue
    variable = temperature
    #execute_on = 'initial timestep_begin'
  []
  [general_sensor_pp]
    type = GeneralSensorPostprocessor
    execute_on = 'initial timestep_end'
    input_signal = input_signal_pp
    noise_std_dev_function = '0'
    delay_function = delay_func
    drift_function = '0'
    efficiency_function = '1'
    signalToNoise_function = '0'
    uncertainty_std_dev_function = '0'
    R_function = '0'
    proportional_weight = '1'
    integral_weight = '1'
    seed = 999
  []
[]

[Executioner]
  type = Transient
  line_search = none
  dt = 0.01
  num_steps = 200
  nl_rel_tol = 1e-02
  nl_abs_tol = 1e-8
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  exodus = false
[]
