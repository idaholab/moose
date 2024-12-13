[Problem]
  kernel_coverage_check = false
  solve = false
[]
[Executioner]
  type = Steady
[]
[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[MultiApps]
  [model_grad]
    type = FullSolveMultiApp
    input_files = model_grad.i
    min_procs_per_app = 1
    #this creates 4 subapps
    positions = '0 0 0
                 0 0 0
                 0 0 0
                 0 0 0'
    cli_args = 'id=1;frequencyHz=1.0
                id=2;frequencyHz=2.0
                id=3;frequencyHz=3.0
                id=4;frequencyHz=4.0'
  []
[]
[Reporters]
  [from_sub_rep]
    type = ConstantReporter
    real_vector_vector_names = 'rec_vec_vec'
    real_vector_vector_values = '0'
    real_vector_names = 'rec_vec'
    real_vector_values = "0."
    outputs = out
  []
  [parameters]
    type = ConstantReporter
    real_vector_names = 'G'
    real_vector_values = '4'
  []
  [objective]
    type = ParsedVectorRealReductionReporter
    name = objective
    reporter_name = from_sub_rep/rec_vec
    initial_value = 0
    expression = 'reduction_value+indexed_value'
  []
  [gradient]
    type = ParsedVectorVectorRealReductionReporter
    name = gradient
    reporter_name = "from_sub_rep/rec_vec_vec"
    initial_value = 0
    expression = 'reduction_value+indexed_value'
  []
[]
[Transfers]
  [setPrameters]
    type = MultiAppReporterTransfer
    to_multi_app = model_grad
    from_reporters = 'parameters/G'
    to_reporters = 'parameters/G'
    execute_on = 'TIMESTEP_BEGIN'
  []
  [ObjectivesGradients]
    type = MultiAppReporterTransfer
    from_multi_app = model_grad
    from_reporters = 'gradient/gradient objective/objective'
    to_reporters = 'from_sub_rep/rec_vec_vec from_sub_rep/rec_vec'
    distribute_reporter_vector = true
  []
[]

[Outputs]
  csv = false
  json = false
  console = false
[]
