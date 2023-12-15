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
    type=VectorOfVectorTestReporter
    name = v_of_v
    vector_of_vectors = '101 201; 102 202; 103 203'
    outputs=none
  []
  [vals]
    type = VectorOfVectorRowSum
    name = sum_v_of_v
    reporter_vector_of_vectors = "dataFromVofV/v_of_v"
  []

  [vectors_to_dot]
    type = ConstantReporter
    real_vector_names = 'vec_a vec_b vec_c vec_d'
    real_vector_values = '1 2 3; 10 20 30; 100 10 1; -1 -2'
    outputs=none
  []
  [innerProduct]
    type = VectorDotProduct
    name = inner
    reporter_names = 'vectors_to_dot/vec_a vectors_to_dot/vec_b vectors_to_dot/vec_c'
    reporter_symbols = 'a b c'
    constant_names = 'constant1 constant2'
    constant_expressions = '10 20'
    expression = '(a+b)*c+constant1+constant2'
  []

  [vector_sum]
    type = VectorSum
    name = sum
    vector= vectors_to_dot/vec_a
    scale = 0.5
  []
[]

[Outputs]
  csv=true
[]
