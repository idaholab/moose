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
    input_files = forward.i
    min_procs_per_app = 1
    positions = '0 0 0
                 0 0 0
                 0 0 0'
    cli_args = 'omega=2.0
                omega=3.0
                omega=5.0'
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
    real_vector_names = 'vals'
    real_vector_values = '0 4'
  []
  [obj_sum]
    type = ParsedVectorRealReductionReporter
    name = value
    reporter_name = from_sub_rep/rec_vec
    initial_value = 0
    expression = 'reduction_value+indexed_value'
  []
  [grad_sum]
    type = ParsedVectorVectorRealReductionReporter
    name = row_sum
    reporter_name = "from_sub_rep/rec_vec_vec"
    initial_value = 0
    expression = 'reduction_value+indexed_value'
  []
[]
[Transfers]
  [setPrameters]
    type = MultiAppReporterTransfer
    to_multi_app = model_grad
    from_reporters = 'parameters/vals'
    to_reporters = 'vals/vals'
    execute_on = 'TIMESTEP_BEGIN'
  []
  [ObjectivesGradients]
    type = MultiAppReporterTransfer
    from_multi_app = model_grad
    from_reporters = 'grad_f/grad_f obj_pp/value'
    to_reporters = 'from_sub_rep/rec_vec_vec from_sub_rep/rec_vec'
    distribute_reporter_vector = true
  []
[]

[Outputs]
  csv = false
  json = false
  console = false
[]
