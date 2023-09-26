[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = temperature
  []
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 900
  [../]
  [./right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 1200
  [../]
[]

[Postprocessors]
  [input_signal_pp]
    type = ElementAverageValue
    variable = temperature
  []
  [thermocouple_sensor_pp]
    type = ThermocoupleSensorPostprocessor
    thermocouple_type = B
    input_signal = input_signal_pp
    noise_mean = 0.1
    noise_std_dev = 0.8
    delay = 0.05
    drift_function = '0.0001*t'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  nl_abs_tol = 1e-8
[]

[Outputs]
  csv = true
[]
