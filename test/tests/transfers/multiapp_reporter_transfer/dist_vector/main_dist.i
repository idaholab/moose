[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    nx = 1
    dim = 1
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters]
  [from_sub_rep]
    type = ConstantReporter
    real_vector_vector_names = 'rec_vec_vec'
    real_vector_vector_values = '10000000.'

    execute_on = INITIAL
    outputs = out
  []
  [main]
    type = TestDeclareReporter
    distributed_vector_name = dis_vec
  []
[]

[Positions]
  [elem]
    type = ElementCentroidPositions
    auto_sort = true
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = 'sub_dist.i'
    positions_objects = elem
    execute_on = TIMESTEP_END
  []
[]
[Transfers]
  active = to_sub
  [to_sub]
    type = MultiAppReporterTransfer
    to_multi_app = sub
    from_reporters = 'main/dis_vec'
    to_reporters = 'sub/value'
    distribute_reporter_vector = true
  []
  [from_sub]
    type = MultiAppReporterTransfer
    from_multi_app = sub
    from_reporters = 'sub_rep/dis_vec'
    to_reporters = 'from_sub_rep/rec_vec_vec '
    distribute_reporter_vector = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
    execute_on = TIMESTEP_END
  []
[]
