[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters]
  active = constant
  [constant]
    type = ConstantReporter
    eigen_vector_name = eigen_vector
    eigen_vector_value = '1 2 3 4 5'
    eigen_matrix_name = eigen_matrix
    eigen_matrix_value = '1 2 3;
                          4 5 6'
  []
  [error]
    type = ConstantReporter
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  json = true
[]
