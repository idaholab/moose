[Mesh/generate]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[VectorPostprocessors]
  [to_sub]
    type = ConstantVectorPostprocessor
    vector_names = 'a b'
    value = '10 10 10 ; 20 20 20'
  []
  [from_sub]
    type = ConstantVectorPostprocessor
    vector_names = 'a b'
    value = '30 30 30; 40 40 40'
  []
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  csv = true
[]
