[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
  [addmd]
    type = AddMetaDataGenerator
    input = gmg
    real_scalar_metadata_names = 'foo'
    real_scalar_metadata_values = '1'
  []
[]

[UserObjects/test]
  type = MeshMetaDataInterfaceTest
[]

[Problem]
  solve = False
[]

[Executioner]
  type = Steady
[]
