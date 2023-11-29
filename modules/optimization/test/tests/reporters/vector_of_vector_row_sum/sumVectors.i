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
[]

[Outputs]
  csv=true
[]
