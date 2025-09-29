[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [dataFromVofV]
    type = VectorOfVectorTestReporter
    name = v_of_v
    vector_of_vectors = '101 201; 102 202; 103 203'
    outputs = none
  []
  [vecvec_sum]
    type = ParsedVectorVectorRealReductionReporter
    name = sum
    vector_of_vector_reporter_name = 'dataFromVofV/v_of_v'
    initial_value = 0
    expression = 'reduction_value+indexed_value'
  []
  [vecvec_sqsum]
    type = ParsedVectorVectorRealReductionReporter
    name = sqsum
    vector_of_vector_reporter_name = 'dataFromVofV/v_of_v'
    initial_value = 0
    expression = 'reduction_value+indexed_value*indexed_value'
  []
  [vecvec_multiply]
    type = ParsedVectorVectorRealReductionReporter
    name = multiply
    vector_of_vector_reporter_name = 'dataFromVofV/v_of_v'
    initial_value = 1
    expression = 'reduction_value*indexed_value'
  []
  [vecvec_max]
    type = ParsedVectorVectorRealReductionReporter
    name = max
    vector_of_vector_reporter_name = 'dataFromVofV/v_of_v'
    initial_value = -100000
    expression = 'max(reduction_value,indexed_value)'
  []

  [vecs]
    type = ConstantReporter
    real_vector_names = 'vec_a vec_b vec_c vec_d'
    real_vector_values = '1 2 3; 10 20 30; 100 10 1; 1 2 3 4'
    real_names = 'a b c'
    real_values = '1 10 100'
    outputs = none
  []
  [vectorOperation]
    type = ParsedVectorReporter
    name = inner
    vector_reporter_names = 'vecs/vec_a vecs/vec_b vecs/vec_c'
    vector_reporter_symbols = 'a b c'
    scalar_reporter_names = 'scalar_pp/value'
    scalar_reporter_symbols = 'scalar_pp'
    constant_names = 'constant1 constant2'
    constant_expressions = '10 20'
    expression = '(a+b)*c+constant1+constant2+scalar_pp'
  []
  [scalarOperation]
    type = ParsedScalarReporter
    name = inner
    scalar_reporter_names = 'vecs/a vecs/b vecs/c'
    scalar_reporter_symbols = 'a b c'
    constant_names = 'constant1 constant2'
    constant_expressions = '10 20'
    expression = '(a+b)*c+constant1+constant2'
  []

  [vec_d]
    type = ConstantReporter
    real_vector_names = 'vec_d'
    real_vector_values = '1 2 3 4'
    outputs = none
  []
  [vector_sum]
    type = ParsedVectorRealReductionReporter
    name = sum
    vector_reporter_name = vec_d/vec_d
    initial_value = 0
    expression = 'reduction_value+indexed_value'
  []
  [vector_sqsum]
    type = ParsedVectorRealReductionReporter
    name = sqsum
    vector_reporter_name = vec_d/vec_d
    initial_value = 0
    expression = 'reduction_value+indexed_value*indexed_value'
  []
  [vector_multiply]
    type = ParsedVectorRealReductionReporter
    name = multiply
    vector_reporter_name = vec_d/vec_d
    initial_value = 1
    expression = 'reduction_value*indexed_value'
  []
  [vector_max]
    type = ParsedVectorRealReductionReporter
    name = max
    vector_reporter_name = vec_d/vec_d
    initial_value = -100000
    expression = 'max(reduction_value,indexed_value)'
  []
[]

[Postprocessors]
  [scalar_pp]
    type = ConstantPostprocessor
    value = 5
    outputs = none
  []
[]

[Outputs]
  csv = true
[]
