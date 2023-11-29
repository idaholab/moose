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
  name = data
  vector_of_vectors = '101 102; 103 104; 105 106'
[]
  [vals]
    type = ReporterVectorSum
    summed_vector_name = sum
    reporter_vector_of_vectors = "dataFromVofV/name"
  []
[]

[Outputs]
  json = true
[]
[Debug]
show_reporters = true
[]
