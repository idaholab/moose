[StochasticTools]
[]

[FormFunction]
  type = ObjectiveMinimize
  parameter_vpp = 'parameter_results'
  data_computed = 'data_rec_0 data_rec_1 data_rec_2 data_rec_3'
  data_target = '204 216 167 128'
[]

[Executioner]
  type = Optimize
  tao_solver = TAONM
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
[]

[Transfers]
  [toforward]
    type = OptimizationParameterTransfer
    multi_app = forward
    parameter_vpp = parameter_results
    to_control = parameterReceiver
  []
  [pp_transfer_0]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = data_pt_0
    to_postprocessor = data_rec_0
    reduction_type = average
  []
  [pp_transfer_1]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = data_pt_1
    to_postprocessor = data_rec_1
    reduction_type = average
  []
  [pp_transfer_2]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = data_pt_2
    to_postprocessor = data_rec_2
    reduction_type = average
  []
  [pp_transfer_3]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = data_pt_3
    to_postprocessor = data_rec_3
    reduction_type = average
  []
[]

[VectorPostprocessors]
  [parameter_results]
    type = OptimizationParameterVectorPostprocessor
    parameters = 'Postprocessors/p1/value'
  []
[]

[Postprocessors]
  [data_rec_0]
    type = Receiver
  []
  [data_rec_1]
    type = Receiver
  []
  [data_rec_2]
    type = Receiver
  []
  [data_rec_3]
    type = Receiver
  []
[]


[Outputs]
  console = true
  csv=true
[]
