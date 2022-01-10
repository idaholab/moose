[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = halfSphere.e
  []
  [patch]
    type = PatchSidesetGenerator
    boundary = flat
    n_patches = 20
    input = fmg
  []
[]

[Variables]
  [temperature]
  []
[]

[AuxVariables]
  [saved_t]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
    save_in = saved_t
  []
[]

[BCs]
  [round]
    type = ConvectiveFluxFunction
    boundary = round
    variable = temperature
    coefficient = 0.05
    T_infinity = 100.0
  []
  [flat_0]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_0
    postprocessor = p0
  []
  [flat_1]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_1
    postprocessor = p1
  []
  [flat_2]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_2
    postprocessor = p2
  []
  [flat_3]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_3
    postprocessor = p3
  []
  [flat_4]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_4
    postprocessor = p4
  []
  [flat_5]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_5
    postprocessor = p5
  []
  [flat_6]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_6
    postprocessor = p6
  []
  [flat_7]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_7
    postprocessor = p7
  []
  [flat_8]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_8
    postprocessor = p8
  []
  [flat_9]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_9
    postprocessor = p9
  []
  [flat_10]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_10
    postprocessor = p10
  []
  [flat_11]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_11
    postprocessor = p11
  []
  [flat_12]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_12
    postprocessor = p12
  []
  [flat_13]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_13
    postprocessor = p13
  []
  [flat_14]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_14
    postprocessor = p14
  []
  [flat_15]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_15
    postprocessor = p15
  []
  [flat_16]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_16
    postprocessor = p16
  []
  [flat_17]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_17
    postprocessor = p17
  []
  [flat_18]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_18
    postprocessor = p18
  []
  [flat_19]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = flat_19
    postprocessor = p19
  []
[]

[Postprocessors]
  [p0]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p1]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p2]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p3]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p4]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p5]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p6]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p7]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p8]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p9]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p10]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p11]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p12]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p13]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p14]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p15]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p16]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p17]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p18]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p19]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Problem]#do we need this
  type = FEProblem
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  #-----every forward problem should have these two
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
  []
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  console = true #false
  exodus = true
  csv = true
  file_base = 'forward'
[]
