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
  type = ParameterMeshOptimization
  parameter_names = 'parameter1 parameter2'
  parameter_meshes = 'parameter_mesh_boundsIC_out.e parameter_mesh_boundsIC_out.e'
  parameter_families = 'LAGRANGE MONOMIAL'
  parameter_orders = 'FIRST CONSTANT'
  num_parameter_times=1
  measurement_points = '0.1 0.2 0.3
                        0.4 0.5 0.6
                        0.7 0.8 0.9
                        1.0 1.1 1.2'
  measurement_values = '11 12 13 14'
  outputs = outjson
[]


[UserObjects]
  [optReporterTester]
    type = OptimizationReporterTest
    values_to_set_parameters_to = '10 20 30 40 50 60 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    values_to_set_simulation_measurements_to = '111 212 313 314'
    expected_objective_value = 115000
    expected_lower_bounds = '0 0.5 0.5 0 1 1 0 1 1 0 2 2 0 1.5 1.5 0 3 3 0.25 0.75 0.5 1.5 0.75 2.25'
    expected_upper_bounds = '2 2.5 2.5 2 3 3 2 3 3 2 4 4 2 3.5 3.5 2 5 5 2.25 2.75 2.5 3.5 2.75 4.25'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [outjson]
    type = JSON
    execute_system_information_on = none
  []
[]
