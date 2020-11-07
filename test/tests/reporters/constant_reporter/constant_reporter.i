[Mesh/mesh]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters]
  active = constant
  [constant]
    type = ConstantReporter
    names          = 'int_1   str    int_2   num_1 num_2 vec_1  int_3   vec_2'
    value_types    = 'integer string integer real  real  vector integer vector'
    integer_values = '1 3 -7'
    real_values    = '4.0 5.0'
    vector_values  = '6.0 60.0 600.0; 8.0 80.0'
    string_values  = 'two'
  []
  [error]
    type = ConstantReporter
    names = 'int num vec str'
    value_types = 'integer real vector string'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
