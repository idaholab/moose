[Mesh/generate]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[VectorPostprocessors]
  [from_main_vpp]
    type = ConstantVectorPostprocessor
    vector_names = 'a b c'
    value = '1 1 1; 2 2 2; 3 3 3'
    execute_on = initial
    #outputs = none
  []
  [to_main_vpp]
    type = ConstantVectorPostprocessor
    vector_names = 'a b c'
    value = '4 4 4; 5 5 5; 6 6 6'
    #outputs = none
  []
[]

[Reporters]
  [from_main_rep]
    type = ConstantReporter
    integer_names = int
    integer_values = 1
    real_names = num
    real_values = 2.0
    real_vector_names = vec
    real_vector_values = '3 4'
    string_names = str
    string_values = 'five'
  []
  [to_main_rep]
    type = ConstantReporter
    integer_names = int
    integer_values = 0
    real_names = num
    real_values = 0.0
    real_vector_names = vec
    real_vector_values = '0'
    string_names = str
    string_values = 'foo'
  []
[]

[MultiApps/sub]
  type = TransientMultiApp
  input_files = 'sub0.i sub1.i'
  positions = '0 0 0 0 0 0'
[]

[Transfers]
  # VPP transfers
  [vpp_to_vpp]
    type = MultiAppReporterTransfer
    to_reporters = 'to_sub_vpp/a to_sub_vpp/b'
    from_reporters = 'from_main_vpp/a from_main_vpp/b'
    to_multi_app = sub
  []
  [vpp_from_vpp]
    type = MultiAppReporterTransfer
    to_reporters = 'to_main_vpp/a to_main_vpp/b'
    from_reporters = 'from_sub_vpp/a from_sub_vpp/b'
    from_multi_app = sub
    subapp_index = 0
  []

  # Vector-VPP transfers
  [vector_to_vpp]
    type = MultiAppReporterTransfer
    to_reporters = 'to_sub_vpp/a'
    from_reporters = 'from_main_rep/vec'
    to_multi_app = sub
    subapp_index = 0
  []
  [vector_from_vpp]
    type = MultiAppReporterTransfer
    to_reporters = 'to_main_rep/vec'
    from_reporters = 'from_sub_vpp/a'
    from_multi_app = sub
    subapp_index = 0
  []

  # Real-PP transfers
  [real_to_pp]
    type = MultiAppReporterTransfer
    to_reporters = 'to_sub_pp/value'
    from_reporters = 'from_main_rep/num'
    to_multi_app = sub
    subapp_index = 0
  []
  [real_from_pp]
    type = MultiAppReporterTransfer
    to_reporters = 'to_main_rep/num'
    from_reporters = 'from_sub_pp/value'
    from_multi_app = sub
    subapp_index = 0
  []

  # Int-Int transfers
  [int_to_int]
    type = MultiAppReporterTransfer
    to_reporters = 'to_sub_rep/int'
    from_reporters = 'from_main_rep/int'
    to_multi_app = sub
    subapp_index = 0
  []
  [int_from_int]
    type = MultiAppReporterTransfer
    to_reporters = 'to_main_rep/int'
    from_reporters = 'from_sub_rep/int'
    from_multi_app = sub
    subapp_index = 0
  []

  # String-String transfers
  [string_to_string]
    type = MultiAppReporterTransfer
    to_reporters = 'to_sub_rep/str'
    from_reporters = 'from_main_rep/str'
    to_multi_app = sub
    subapp_index = 0
  []
  [string_from_string]
    type = MultiAppReporterTransfer
    to_reporters = 'to_main_rep/str'
    from_reporters = 'from_sub_rep/str'
    from_multi_app = sub
    subapp_index = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  [out]
    type = JSON
    vectorpostprocessors_as_reporters = true
  []
  execute_on = timestep_end
[]
