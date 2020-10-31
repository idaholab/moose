[StochasticTools]
[]

[FormFunction]
  type = ObjectiveMinimize
  parameter_vpp = 'parameter_results'
  data_computed = 'data_rec_0 data_rec_1 data_rec_2 data_rec_3'
  data_target = '100 204 320 216'
[]

[Executioner]
  type = Optimize
  tao_solver = TAONM
  verbose = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    execute_on = nonlinear # really important to make subapp execute every optimization step
    input_files = sub.i
  []
[]

[Transfers]
  [toSub]
    type = OptimizationParameterTransfer
    multi_app = sub
    parameter_vpp = parameter_results
    to_control = parameterReceiver

    #get reference from form function get parameter_names.  Push them into control receiver.


    # this will control the values directly, and get initial values and put them in vpp
    # should use somekind of key value storage object  Need to ask Daniel for clarification this is some kind of userobject.
  []
  [pp_transfer_0]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = sub
    from_postprocessor = data_pt_0
    to_postprocessor = data_rec_0
    reduction_type = average
  []
  [pp_transfer_1]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = sub
    from_postprocessor = data_pt_1
    to_postprocessor = data_rec_1
    reduction_type = average
  []
  [pp_transfer_2]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = sub
    from_postprocessor = data_pt_2
    to_postprocessor = data_rec_2
    reduction_type = average
  []
  [pp_transfer_3]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = sub
    from_postprocessor = data_pt_3
    to_postprocessor = data_rec_3
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
