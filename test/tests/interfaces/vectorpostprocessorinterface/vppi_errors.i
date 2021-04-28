[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/error_test]
  type = VectorPostprocessorInterfaceErrorTest
  vpp = constant_vpp
[]

[VectorPostprocessors/constant_vpp]
  type = ConstantVectorPostprocessor
  vector_names = 'vpp_val'
  value = '1'
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
