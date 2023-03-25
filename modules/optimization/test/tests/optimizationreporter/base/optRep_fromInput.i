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
  upper_bounds = '110; 210; 310'
  lower_bounds = '-1; -2; -3'
  measurement_points = '0.1 0.2 0.3
                        0.4 0.5 0.6
                        0.7 0.8 0.9
                        1.0 1.1 1.2'
  measurement_values = '11 12 13 14'
  outputs = out
[]

[UserObjects]
  [optReporterTester]
    type = OptimizationReporterTest
    values_to_set_parameters_to = '10 20 30 40 50 60 70 80 90 100 110 120'
    values_to_set_simulation_measurements_to = '111 212 313 314'
    expected_objective_value = 115000
    expected_lower_bounds = '-1 -1 -2 -2 -2 -2 -3 -3 -3 -3 -3 -3'
    expected_upper_bounds = '110 110 210 210 210 210 310 310 310 310 310 310'
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
