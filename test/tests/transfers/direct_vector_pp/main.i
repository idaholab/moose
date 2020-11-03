[Mesh/generate]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[VectorPostprocessors]
  [from_main]
    type = ConstantVectorPostprocessor
    vector_names = 'a b c'
    value = '1 1 1; 2 2 2; 3 3 3'
  []
  [to_main]
    type = ConstantVectorPostprocessor
    vector_names = 'a b c'
    value = '4 4 4; 5 5 5; 6 6 6'
  []
[]

[MultiApps/sub]
  type = TransientMultiApp
  input_files = 'sub0.i sub1.i'
  positions = '0 0 0 0 0 0'
[]

[Transfers]
  [send]
    type = MultiAppDirectVectorPostprocessorTransfer
    to_vector_postprocessor = to_sub
    from_vector_postprocessor = from_main
    vector_names = 'a b'
    direction = to_multiapp
    multi_app = sub
  []
  [receive]
    type = MultiAppDirectVectorPostprocessorTransfer
    to_vector_postprocessor = to_main
    from_vector_postprocessor = from_sub
    vector_names = 'a b'
    direction = from_multiapp
    multi_app = sub
    subapp_index = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
