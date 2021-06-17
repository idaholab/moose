[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables/u]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Reporters]
  [constant]
    type = ConstantReporter
    integer_vector_names  = 'int_vec'
    integer_vector_values = '1 2 3 4'

    real_vector_names  = 'vec_1                  vec_2'
    real_vector_values = '5.0 50.0 500.0 5000.0; 6.6 66.6 666.6 6666.6'

    string_vector_names  = 'str_vec'
    string_vector_values = 'seven eight nine ten'

    integer_names = int
    integer_values = 11

    real_names = num
    real_values = 12.1

    string_names = str
    string_values = thirteen
  []
  [test]
    type = TestDeclareReporter
  []
[]

[Postprocessors]
  [numdofs]
    type = NumDOFs
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  csv = true
[]
