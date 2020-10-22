[StochasticTools]
[]

[FormFunction]
  type = ObjectiveMinimize
  optimization_vpp = 'opt_results' # parameters : these are the parameters being controlled
  subapp_vpp = 'vpp_receiver' # simulated_data :       measurement data from teh simulation
  measurement_vpp='measurements'  # measured_data :   converged on measured data
[]

[Executioner]
  type = Optimize
  petsc_options_iname = '-tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type'
  petsc_options_value = '0 1e16 constant'
  solve_on = none
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
    type = OptimizationTransfer #parameterTransfer : this name could be better
    multi_app = sub
    optimization_vpp = opt_results #paramter_vpp :
    to_control = optimizationReceiver #parameterReceiver
  []
  [fromSub]
    type = MultiAppVppToVppTransfer
    vector_postprocessor_sub = 'measure_pts'
    vector_postprocessor_master = 'vpp_receiver'
    direction = from_multiapp
    multi_app = sub
  []
[]

[VectorPostprocessors]
  [measurements]
    type = ConstantVectorPostprocessor #optDataVectorPostprocessor
    value = '100 204 320 216'
    vector_names = 'values'
  []
  [opt_results]
    type = OptimizationVectorPostprocessor  #optParameterVectorPostprocessor
    parameters = 'DiracKernels/pt0/value
                  DiracKernels/pt1/value
                  DiracKernels/pt2/value'
    intial_values = '-2458 7257 26335'
  []
  [vpp_receiver]
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
