[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Problem]
  solve = false
[]

[UserObjects]
  [test]
    type = VectorPostprocessorInterfaceErrorTest
    vpp = constant_vpp
    call_getter_too_early = true
  []
[]

[VectorPostprocessors]
  [constant_vpp]
    type = ConstantVectorPostprocessor
    vector_names = 'value'
    value = '1'
  []
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  console = false
[]
