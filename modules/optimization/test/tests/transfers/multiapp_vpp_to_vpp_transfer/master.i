[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 1
  ymax = 2
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./v]
    [./InitialCondition]
      type = FunctionIC
      function = set_v
    [../]
  [../]
[]

[Functions]
  [./set_v]
    type = ParsedFunction
    value = 'x + t'
  [../]
[]

[VectorPostprocessors]
  [./sample_points]
    type = PointValueSampler
    variable = v
    points = '0.25 1.25 0 0.5 1.5 0'
    sort_by = x
    execute_on = 'initial timestep_end'
  [../]

  [./receive_values]
    type = VectorPostprocessorReceiver
    execute_on = timestep_end
  [../]
[]

[MultiApps]
  [./sub]
    type = FullSolveMultiApp
    input_files = 'sub.i'
    max_procs_per_app = 1
  [../]
[]

[Transfers]
  [./send]
    type = MultiAppVppToVppTransfer
    vector_postprocessor_master = sample_points
    vector_postprocessor_sub = receive_values_sub
    direction = to_multiapp
    multi_app = sub
    execute_on = timestep_begin
  [../]
  [./receive]
    type = MultiAppVppToVppTransfer
    vector_postprocessor_sub = csv_reader_sub
    vector_postprocessor_master = receive_values
    direction = from_multiapp
    multi_app = sub
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = master
  csv = true
[]
