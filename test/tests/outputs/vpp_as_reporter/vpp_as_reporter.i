[Mesh/gen]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors/data]
  type = ConstantVectorPostprocessor
  vector_names = 'vector'
  value = '1949 1954 1977 1980'
[]

[Outputs]
  [out]
    type = JSON
    vectorpostprocessors_as_reporters = true
  []
[]
