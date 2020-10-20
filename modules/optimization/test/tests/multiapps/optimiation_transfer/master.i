
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
[]

[Variables]
  [response]
  []
[]

[Kernels]
  [null_kernel]
    type = NullKernel
    variable = response
  []
[]

[FormFunction]
  type = ObjectiveMinimize
  optimization_vpp = 'opt_results'
  subapp_vpp = 'vpp_receiver'
  measurement_vpp='measurements'
[]

[Executioner]
  type = Optimize
  petsc_options_iname = '-tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type'
  petsc_options_value = '0 1e16 constant'
  solve_on = none
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    execute_on = nonlinear
    input_files = sub.i
  []
[]

[Transfers]
  [toSub]
    type = OptimizationTransfer
    multi_app = sub
    optimization_vpp = opt_results
    to_control = optimizationSamplerReceiver
  []
  [fromSub]
    type = MultiAppVppToVppTransfer
    vector_postprocessor_sub = 'measure_pts'
    vector_postprocessor_master = 'vpp_receiver'
    direction = from_multiapp
    multi_app = sub
  []
  [fullTransfer]
    type = MultiAppCopyTransfer
    source_variable = temperature
    variable = response
    direction = from_multiapp
    multi_app = sub
  []
[]

[VectorPostprocessors]
  [measurements]
    type = ConstantVectorPostprocessor
    value = '100 204 320 216'
    vector_names = 'values'
  []
  [opt_results]
    type = OptimizationVectorPostprocessor
    parameters = 'DiracKernels/pt0/value
                  DiracKernels/pt1/value
                  DiracKernels/pt2/value'
    intial_values = '0 0 0'
  []
  [vpp_receiver]
    type=VectorPostprocessorReceiver
  []
[]

[Outputs]
  console = true
  csv=true
  exodus = true
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
