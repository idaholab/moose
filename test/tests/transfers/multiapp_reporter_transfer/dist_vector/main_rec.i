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
  [from_sub_rep]
    type = ConstantReporter
    real_vector_vector_names = 'rec_vec_vec'
    real_vector_vector_values = '10000000.'
    real_vector_names = 'rec_vec'
    real_vector_values = "0."
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
    cli_args = 'Reporters/from_main_rep/real_vector_values="0 1";Reporters/from_main_rep/real_values=0
    Reporters/from_main_rep/real_vector_values="-10";Reporters/from_main_rep/real_values=-5
    Reporters/from_main_rep/real_vector_values="100 -100 -200";Reporters/from_main_rep/real_values=3.33
    Reporters/from_main_rep/real_vector_values="5.55 5.55 5.55 5.55";Reporters/from_main_rep/real_values=7'

  []
[]
[Transfers]
  [from_sub]
    type = MultiAppReporterTransfer
    from_multi_app = sub
    from_reporters = 'from_main_rep/rec_vec from_main_rep/rec_real'
    to_reporters = 'from_sub_rep/rec_vec_vec from_sub_rep/rec_vec'
    distribute_reporter_vector = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = N_to_1
  [out]
    type = JSON
    execute_system_information_on = NONE
    execute_on = TIMESTEP_END
  []
[]
