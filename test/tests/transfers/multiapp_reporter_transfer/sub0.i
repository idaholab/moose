[Mesh/generate]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Postprocessors]
  [to_sub_pp]
    type = Receiver
  []
  [from_sub_pp]
    type = Receiver
    default = 3.1415926
  []
[]

[VectorPostprocessors]
  [to_sub_vpp]
    type = ConstantVectorPostprocessor
    vector_names = 'a b'
    value = '10 10 10 ; 20 20 20'
  []
  [from_sub_vpp]
    type = ConstantVectorPostprocessor
    vector_names = 'a b'
    value = '30 30 30; 40 40 40'
  []
[]

[Reporters]
  [to_sub_rep]
    type = ConstantReporter
    integer_names = int
    integer_values = 0
    string_names = str
    string_values = 'foo'
  []
  [from_sub_rep]
    type = ConstantReporter
    integer_names = int
    integer_values = 10
    string_names = str
    string_values = 'twenty'
  []
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  [out]
    type = JSON
    vectorpostprocessors_as_reporters = true
    postprocessors_as_reporters = true
  []
  execute_on = timestep_end
[]
