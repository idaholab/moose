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
    value = '11 11 11 ; 21 21 21'
  []
  [from_sub]
    type = ConstantVectorPostprocessor
    vector_names = 'a b'
    value = '31 31 31; 41 41 41'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
