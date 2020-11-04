[StochasticTools]
[]

[FormFunction]
  type = ObjectiveGradientMinimize
  adjoint_vpp = 'adjoint_results'
  adjoint_data_computed = 'adjoint_rec_0 adjoint_rec_1 adjoint_rec_2'
  parameter_vpp = 'parameter_results'
  data_computed = 'data_rec_0 data_rec_1 data_rec_2 data_rec_3'
  data_target = '100 204 320 216'
[]

[Executioner]
  type = Optimize
  tao_solver = TAOCG
  petsc_options_iname = '-tao_gatol'# -tao_cg_delta_max'
  petsc_options_value = '1e-4'# 1e-2'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
  []
[]

[Transfers]
  #OptimizationParameterTransfer
  #get reference from form function get parameter_names.  Push them into control receiver.

  # this will control the values directly, and get initial values and put them in vpp
  # should use somekind of key value storage object  Need to ask Daniel for clarification
  # this is some kind of userobject.
  [toForward]
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


  [toAdjoint]
    type = OptimizationParameterTransfer
    multi_app = adjoint
    parameter_vpp = adjoint_results
    to_control = adjointReceiver
  []
  [pp_adjoint_0]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = adjoint_pt_0
    to_postprocessor = adjoint_rec_0
    reduction_type = average
  []
  [pp_adjoint_1]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = adjoint_pt_1
    to_postprocessor = adjoint_rec_1
    reduction_type = average
  []
  [pp_adjoint_2]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = adjoint_pt_2
    to_postprocessor = adjoint_rec_2
    reduction_type = average
  []
[]

[VectorPostprocessors]
  [parameter_results]
    type = OptimizationParameterVectorPostprocessor
    parameters = 'DiracKernels/pt0/value
                  DiracKernels/pt1/value
                  DiracKernels/pt2/value'
  []
  [adjoint_results]
    type = OptimizationParameterVectorPostprocessor
    parameters = 'DiracKernels/pt0/value
                  DiracKernels/pt1/value
                  DiracKernels/pt2/value
                  DiracKernels/pt3/value'
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
  [adjoint_rec_0]
    type = Receiver
    outputs = none
  []
  [adjoint_rec_1]
    type = Receiver
    outputs = none
  []
  [adjoint_rec_2]
    type = Receiver
    outputs = none
  []
[]


[Outputs]
  console = true
  csv=true
[]

#action for optimization
# [Postprocessors]
#   # [objective]
#   #   type = VectorPostprocessorDifferenceMeasure
#   #   vectorpostprocessor_a = measurement_data
#   #   vectorpostprocessor_b = computed_data
#   #   vector_name_a = 'measurementData'
#   #   vector_name_b = 'response'
#   #   difference_type = l2
#   #   execute_on = 'timestep_end'
#   #   outputs = fromMaster
#   # []
# []
