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
    expression = 'x + 2 * y'
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
    type = PointValueSampler
    variable = v
    points = '0.25 1.25 0 0.5 1.5 0'
    sort_by = x
    execute_on = initial
  [../]
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    input_files = 'sub.i'
    positions = '0.25 1.25 0 0.5 1.5 0'
    max_procs_per_app = 1
  [../]
[]

[Transfers]
  [./send]
    type = MultiAppVectorPostprocessorTransfer
    vector_postprocessor = sample_points
    postprocessor = receive
    vector_name = v
    to_multi_app = sub
  [../]

  [./receive]
    type = MultiAppVectorPostprocessorTransfer
    vector_postprocessor = receive_values
    postprocessor = send
    vector_name = v
    from_multi_app = sub
  [../]
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-10
  num_steps = 1
[]

[Outputs]
  csv = true
[]
