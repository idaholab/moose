[StochasticTools]
[]

[FormFunction]
  type = ObjectiveVppMinimize
  parameter_vpp = 'parameter_results'
  data_vpp = 'data_receiver'
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
  []
  [fromSub]
    type = MultiAppVppToVppTransfer
    vector_postprocessor_sub = 'measure_pts'
    vector_postprocessor_master = 'data_receiver'
    direction = from_multiapp
    multi_app = sub
  []
[]

[VectorPostprocessors]
  [parameter_results]
    type = OptimizationParameterVectorPostprocessor
    parameters = 'DiracKernels/pt0/value
                  DiracKernels/pt1/value
                  DiracKernels/pt2/value'  []
  [data_receiver]
    type=VectorPostprocessorReceiver
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
