[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    nx = 4
    dim = 1
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters]
  [to_sub_rep]
    type = ConstantReporter
    real_vector_vector_names = 'sent_vec'
    real_vector_vector_values = '1.; 2. 1003.;-5.0 -10 1000;3.3'
    real_vector_names = 'sent_real'
    real_vector_values = "1. 2. 3. 4."
    execute_on = INITIAL
    outputs = out
  []
[]

[Positions]
  [elem]
    type = ElementCentroidPositions
    auto_sort = true
    outputs = none
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = 'sub.i'
    positions_objects = elem
    execute_on = TIMESTEP_END
  []
[]
[Transfers]
  [to_sub]
    type = MultiAppReporterTransfer
    to_multi_app = sub
    from_reporters = 'to_sub_rep/sent_vec to_sub_rep/sent_real'
    to_reporters = 'from_main_rep/rec_vec from_main_rep/rec_real'
    distribute_reporter_vector = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 1_to_N
  [out]
    type = JSON
    execute_system_information_on = NONE
    execute_on = TIMESTEP_END
  []
[]
