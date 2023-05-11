[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [null]
    type = NullKernel
    variable = u
  []
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'p1 p2 p3'
  num_values = '2 4 6'
  initial_condition = '1 2; 3 4 5 6; 7 8 9 10 11 12'
  upper_bounds = '101 102; 103 104 105 106; 107 108 109 110 111 112'
  lower_bounds = '-1 -2; -3 -4 -5 -6; -7 -8 -9 -10 -11 -12'
  measurement_file = 'measurementData.csv'
  file_xcoord = 'coordx'
  file_ycoord ='y'
  file_zcoord = 'z'
  file_value = 'measured_value'
  outputs = out
[]

[UserObjects]
  [optReporterTester]
    type = OptimizationReporterTest
    values_to_set_parameters_to = '10 20 30 40 50 60 70 80 90 100 110 120'
    values_to_set_simulation_measurements_to = '111 212 313 314'
    expected_objective_value = 115000
    expected_lower_bounds = '-1 -2 -3 -4 -5 -6 -7 -8 -9 -10 -11 -12'
    expected_upper_bounds = '101 102 103 104 105 106 107 108 109 110 111 112'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
