[Mesh]
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
[]

[BCs]
  [left_constant]
    type = FunctionNeumannBC
    variable = temperature
    boundary = left
    function = left_constant_function
  []
  [left_linear]
    type = FunctionNeumannBC
    variable = temperature
    boundary = left
    function = left_linear_function
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 100
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 200
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 100
  []
[]

[Functions]
  [left_constant_function]
    type = ParsedFunction
    value = a*1.0
    vars = 'a'
    vals = 'p1'
  []
  [left_linear_function]
    type = ParsedFunction
    value = b*y
    vars = 'b'
    vals = 'p2'
  []
[]

[Materials]
  [steel]
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[VectorPostprocessors]
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
  []
   [vertical_1]
     type = LineValueSampler
     variable = 'temperature'
     start_point = '0.2 0.0 0'
     end_point = '0.2 2.0 0'
     num_points = 21
     sort_by = y
   [../]
   [vertical_2]
     type = LineValueSampler
     variable = 'temperature'
     start_point = '0.8 0.0 0'
     end_point = '0.8 2.0 0'
     num_points = 21
     sort_by = y
   [../]
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]

[Postprocessors]
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
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'forward'
[]
