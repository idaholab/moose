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
  #user kernel coverage = no in PROBLEM
  [null_kernel]
    type = NullKernel
    variable = response
  []
[]

[FormFunction]
  type = QuadraticMinimize
  optimization_vpp = 'opt_results'
  objective = 1.0
  solution = '1 2 3'
[]

[Executioner]
  type = Optimize
  petsc_options_iname = '-tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type'
  petsc_options_value = '0 1e16 constant'
  solve_on = none
[]

[MultiApps]
  [full_solve]
    type = FullSolveMultiApp
    execute_on = timestep_begin
    input_files = sub.i
    clone_master_mesh = true
  []
[]

[Transfers]
  [toSub]
    type = OptimizationTransfer
    multi_app = full_solve
    optimization_vpp = opt_results
    to_control = optimizationSamplerReceiver
  []
  [fromSub]
    type = MultiAppCopyTransfer
    direction = from_multiapp
    source_variable = temperature
    variable = response
    multi_app = full_solve
  []
[]

[VectorPostprocessors]
  [computed_data]
    type = PointValueSampler
    variable = 'response'
    points = '0.25 0.5 0
              0.75 0.5 0
              0.25 1.5 0
              0.75 1.5 0'
    sort_by = id
    outputs = none
  []

  [opt_results]
    type = OptimizationVectorPostprocessor
    parameters = 'DiracKernels/pt0/value
    DiracKernels/pt1/value
    DiracKernels/pt2/value'
    intial_values = '100 200 300'
    outputs = optout
  []
[]

[Outputs]
  console = true
  [exodus]
    file_base = 'zmaster/out'
    type = Exodus
  []
  [optout]
    type=CSV
  []
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
