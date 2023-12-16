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
  [vecvec_sum]
    type = VectorOfVectorRowSum
    name = sum
    reporter_vector_of_vectors= 'dataFromVofV/v_of_v'
    initial_value = 0
    expression = 'vi+vplus'
  []
  [vecvec_sqsum]
    type = VectorOfVectorRowSum
    name = sqsum
    reporter_vector_of_vectors= 'dataFromVofV/v_of_v'
    initial_value = 0
    expression = 'vi+vplus*vplus'
  []
  [vecvec_multiply]
    type = VectorOfVectorRowSum
    name = multiply
    reporter_vector_of_vectors= 'dataFromVofV/v_of_v'
    initial_value = 1
    expression = 'vi*vplus'
  []
  [vecvec_max]
    type = VectorOfVectorRowSum
    name = max
    reporter_vector_of_vectors= 'dataFromVofV/v_of_v'
    initial_value = -100000
    expression = 'max(vi,vplus)'
  []

  [vecs]
    type = ConstantReporter
    real_vector_names = 'vec_a vec_b vec_c vec_d'
    real_vector_values = '1 2 3; 10 20 30; 100 10 1; 1 2 3 4'
    outputs=none
  []
  [vectorOperation]
    type = VectorDotProduct
    name = inner
    reporter_names = 'vecs/vec_a vecs/vec_b vecs/vec_c'
    reporter_symbols = 'a b c'
    constant_names = 'constant1 constant2'
    constant_expressions = '10 20'
    expression = '(a+b)*c+constant1+constant2'
  []

  [vec_d]
    type = ConstantReporter
    real_vector_names = 'vec_d'
    real_vector_values = '1 2 3 4'
    outputs=none
  []
  [vector_sum]
    type = VectorSum
    name = sum
    reporter_name= vec_d/vec_d
    initial_value = 0
    expression = 'vi+vplus'
  []
  [vector_sqsum]
    type = VectorSum
    name = sqsum
    reporter_name= vec_d/vec_d
    initial_value = 0
    expression = 'vi+vplus*vplus'
  []
  [vector_multiply]
    type = VectorSum
    name = multiply
    reporter_name= vec_d/vec_d
    initial_value = 1
    expression = 'vi*vplus'
  []
  [vector_max]
    type = VectorSum
    name = max
    reporter_name= vec_d/vec_d
    initial_value = -100000
    expression = 'max(vi,vplus)'
  []
[]

[Outputs]
  csv=true
[]
