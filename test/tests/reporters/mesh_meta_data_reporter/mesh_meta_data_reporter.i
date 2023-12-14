[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
  [metadata]
    type = AddMetaDataGenerator
    input = gmg
    uint_vector_metadata_names = 'foo'
    uint_vector_metadata_values = '1 2 3 4'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters/metadata]
  type = MeshMetaDataReporter
[]

[Outputs]
  [out]
    type = JSON
    execute_on = initial
    execute_system_information_on = none
  []
[]
