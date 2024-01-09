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
  objective_name = objective_value
  parameter_names = 'parameter'
  parameter_families = 'LAGRANGE'
  parameter_orders = 'FIRST'
  parameter_meshes = parameter_mesh_boundsIC_out.e
  outputs = outjson
[]

[UserObjects]
  [optReporterTester]
    type = OptimizationReporterTest
    values_to_set_parameters_to = '10 20 30 40 50 60 0 0 0 0 0 0 0 0 0 0 0 0'
    expected_lower_bounds = '0 0.5 0.5 0 1 1 0 1 1 0 2 2 0 1.5 1.5 0 3 3'
    expected_upper_bounds = '2 2.5 2.5 2 3 3 2 3 3 2 4 4 2 3.5 3.5 2 5 5'
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
