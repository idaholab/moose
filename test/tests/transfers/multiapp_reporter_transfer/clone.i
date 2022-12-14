[Mesh/generate]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters]
  [receiver]
    type = ConstantReporter
  []
[]

[MultiApps]
  [multi_vector]
    type = TransientMultiApp
    input_files = 'sub0.i sub0.i sub1.i sub1.i'
    positions = '0 0 0
                 0 0 0
                 0 0 0
                 0 0 0'
    cli_args = 'Outputs/active="" Outputs/active="" Outputs/active="" Outputs/active=""'
  []
  [multi_reporter]
    type = TransientMultiApp
    input_files = 'sub0.i sub0.i sub0.i sub0.i'
    positions = '0 0 0
                 0 0 0
                 0 0 0
                 0 0 0'
    cli_args = 'Postprocessors/from_sub_pp/default=3.1415926;Reporters/from_sub_rep/integer_values=10;Reporters/from_sub_rep/string_values=ten;Outputs/active=""
                Postprocessors/from_sub_pp/default=1.5707963;Reporters/from_sub_rep/integer_values=11;Reporters/from_sub_rep/string_values=twenty;Outputs/active=""
                Postprocessors/from_sub_pp/default=1.0471975;Reporters/from_sub_rep/integer_values=12;Reporters/from_sub_rep/string_values=thirty;Outputs/active=""
                Postprocessors/from_sub_pp/default=0.7853981;Reporters/from_sub_rep/integer_values=13;Reporters/from_sub_rep/string_values=forty;Outputs/active=""'
  []
  [single_app]
    type = TransientMultiApp
    input_files = 'sub0.i'
    cli_args = 'Outputs/active=""'
  []
[]

[Transfers]
  [multi_vpp]
    type = MultiAppCloneReporterTransfer
    from_reporters = 'from_sub_vpp/a from_sub_vpp/b'
    to_reporter = receiver
    from_multi_app = multi_vector
  []
  [multi_rep]
    type = MultiAppCloneReporterTransfer
    from_reporters = 'from_sub_pp/value from_sub_rep/int from_sub_rep/str'
    to_reporter = receiver
    from_multi_app = multi_reporter
  []
  [single]
    type = MultiAppCloneReporterTransfer
    from_reporters = 'from_sub_pp/value from_sub_rep/int from_sub_rep/str'
    to_reporter = receiver
    from_multi_app = single_app
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
