[StochasticTools]
[]

[FormFunction]
  type = ObjectiveGradientMinimize
  adjoint_vpp = 'adjoint_results'
  adjoint_data_computed = 'ar00 ar01 ar02 ar03 ar04 ar05 ar06 ar07'
  parameter_vpp = 'parameter_results'
  data_computed = 'dr00 dr01 dr02 dr03 dr04 dr05 dr06 dr07 dr08 dr09 dr10 dr11 dr12 dr13 dr14 dr15 dr16 dr17 dr18 dr19 dr20 dr21 dr22 dr23 dr24 dr25 dr26 '
  data_target = '6.65 12.04 15.13 16.17 15.50 13.65 10.89 7.57 3.87 1.88 3.41 4.26 4.55 4.36 3.84 3.07 2.14 1.09 3.00 5.38 6.61 6.93 6.52 5.66 4.47 3.09 1.57 '
[]

[Executioner]
  type = Optimize
  # tao_solver = taonm
  # petsc_options_iname='-tao_gatol'
  # petsc_options_value='1e-2'
  tao_solver = taolmvm #TAOOWLQN #TAOBMRM #taolmvm #taocg
  petsc_options_iname = '-tao_gatol'# -tao_cg_delta_max'
  petsc_options_value = '1e-2'
  # tao_solver = taontr
  # petsc_options_iname='-tao_fd_hessian -tao_fd_delta -tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type -tao_gatol'
  # petsc_options_value='true 0.000001 0 1e16 constant 1e-2'
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
    from_postprocessor = dr00
    to_postprocessor = dr00
    reduction_type = average
  []
  [pp_transfer_1]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr01
    to_postprocessor = dr01
    reduction_type = average
  []
  [pp_transfer_2]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr02
    to_postprocessor = dr02
    reduction_type = average
  []
  [pp_transfer_3]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr03
    to_postprocessor = dr03
    reduction_type = average
  []
  [pp_transfer_4]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr04
    to_postprocessor = dr04
    reduction_type = average
  []
  [pp_transfer_5]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr05
    to_postprocessor = dr05
    reduction_type = average
  []
  [pp_transfer_6]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr06
    to_postprocessor = dr06
    reduction_type = average
  []
  [pp_transfer_7]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr07
    to_postprocessor = dr07
    reduction_type = average
  []
  [pp_transfer_8]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr08
    to_postprocessor = dr08
    reduction_type = average
  []
  [pp_transfer_9]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr09
    to_postprocessor = dr09
    reduction_type = average
  []
  [pp_transfer_10]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr10
    to_postprocessor = dr10
    reduction_type = average
  []
  [pp_transfer_11]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr11
    to_postprocessor = dr11
    reduction_type = average
  []
  [pp_transfer_12]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr12
    to_postprocessor = dr12
    reduction_type = average
  []
  [pp_transfer_13]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr13
    to_postprocessor = dr13
    reduction_type = average
  []
  [pp_transfer_14]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr14
    to_postprocessor = dr14
    reduction_type = average
  []
  [pp_transfer_15]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr15
    to_postprocessor = dr15
    reduction_type = average
  []
  [pp_transfer_16]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr16
    to_postprocessor = dr16
    reduction_type = average
  []
  [pp_transfer_17]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr17
    to_postprocessor = dr17
    reduction_type = average
  []
  [pp_transfer_18]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr18
    to_postprocessor = dr18
    reduction_type = average
  []
  [pp_transfer_19]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr19
    to_postprocessor = dr19
    reduction_type = average
  []
  [pp_transfer_20]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr20
    to_postprocessor = dr20
    reduction_type = average
  []
  [pp_transfer_21]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr21
    to_postprocessor = dr21
    reduction_type = average
  []
  [pp_transfer_22]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr22
    to_postprocessor = dr22
    reduction_type = average
  []
  [pp_transfer_23]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr23
    to_postprocessor = dr23
    reduction_type = average
  []
  [pp_transfer_24]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr24
    to_postprocessor = dr24
    reduction_type = average
  []
  [pp_transfer_25]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr25
    to_postprocessor = dr25
    reduction_type = average
  []
  [pp_transfer_26]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = forward
    from_postprocessor = dr26
    to_postprocessor = dr26
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
    from_postprocessor = ar00
    to_postprocessor = ar00
    reduction_type = average
  []
  [pp_adjoint_1]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = ar01
    to_postprocessor = ar01
    reduction_type = average
  []
  [pp_adjoint_2]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = ar02
    to_postprocessor = ar02
    reduction_type = average
  []
  [pp_adjoint_3]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = ar03
    to_postprocessor = ar03
    reduction_type = average
  []
  [pp_adjoint_4]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = ar04
    to_postprocessor = ar04
    reduction_type = average
  []
  [pp_adjoint_5]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = ar05
    to_postprocessor = ar05
    reduction_type = average
  []
  [pp_adjoint_6]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = ar06
    to_postprocessor = ar06
    reduction_type = average
  []
  [pp_adjoint_7]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = adjoint
    from_postprocessor = ar07
    to_postprocessor = ar07
    reduction_type = average
  []
[]

[VectorPostprocessors]
  [parameter_results]
    type = OptimizationParameterVectorPostprocessor
    parameters = 'DiracKernels/ar00/value
                  DiracKernels/ar01/value
                  DiracKernels/ar02/value
                  DiracKernels/ar03/value
                  DiracKernels/ar04/value
                  DiracKernels/ar05/value
                  DiracKernels/ar06/value
                  DiracKernels/ar07/value'
  []
  [adjoint_results]
    type = OptimizationParameterVectorPostprocessor
    parameters = 'DiracKernels/a00/value
                  DiracKernels/a01/value
                  DiracKernels/a02/value
                  DiracKernels/a03/value
                  DiracKernels/a04/value
                  DiracKernels/a05/value
                  DiracKernels/a06/value
                  DiracKernels/a07/value
                  DiracKernels/a08/value
                  DiracKernels/a09/value
                  DiracKernels/a10/value
                  DiracKernels/a11/value
                  DiracKernels/a12/value
                  DiracKernels/a13/value
                  DiracKernels/a14/value
                  DiracKernels/a15/value
                  DiracKernels/a16/value
                  DiracKernels/a17/value
                  DiracKernels/a18/value
                  DiracKernels/a19/value
                  DiracKernels/a20/value
                  DiracKernels/a21/value
                  DiracKernels/a22/value
                  DiracKernels/a23/value
                  DiracKernels/a24/value
                  DiracKernels/a25/value
                  DiracKernels/a26/value'
  []
[]

[Postprocessors]
  [dr00]
    type = Receiver
  []
  [dr01]
    type = Receiver
  []
  [dr02]
    type = Receiver
  []
  [dr03]
    type = Receiver
  []
  [dr04]
    type = Receiver
  []
  [dr05]
    type = Receiver
  []
  [dr06]
    type = Receiver
  []
  [dr07]
    type = Receiver
  []
  [dr08]
    type = Receiver
  []
  [dr09]
    type = Receiver
  []
  [dr10]
    type = Receiver
  []
  [dr11]
    type = Receiver
  []
  [dr12]
    type = Receiver
  []
  [dr13]
    type = Receiver
  []
  [dr14]
    type = Receiver
  []
  [dr15]
    type = Receiver
  []
  [dr16]
    type = Receiver
  []
  [dr17]
    type = Receiver
  []
  [dr18]
    type = Receiver
  []
  [dr19]
    type = Receiver
  []
  [dr20]
    type = Receiver
  []
  [dr21]
    type = Receiver
  []
  [dr22]
    type = Receiver
  []
  [dr23]
    type = Receiver
  []
  [dr24]
    type = Receiver
  []
  [dr25]
    type = Receiver
  []
  [dr26]
    type = Receiver
  []

  [ar00]
    type = Receiver
    #outputs = none
  []
  [ar01]
    type = Receiver
  []
  [ar02]
    type = Receiver
  []
  [ar03]
    type = Receiver
  []
  [ar04]
    type = Receiver
  []
  [ar05]
    type = Receiver
  []
  [ar06]
    type = Receiver
  []
  [ar07]
    type = Receiver
  []
[]


[Outputs]
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
  print_linear_residuals = false
  #console = false
  #console = true
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
