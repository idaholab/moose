[Mesh]
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = temperature
    diffusivity = thermal_conductivity
  []
[]

[BCs]
  [left]
    type = FunctionNeumannBC
    variable = temperature
    boundary = left
    function = left_function
  []
  [right]
    type = FunctionNeumannBC
    variable = temperature
    boundary = right
    function = right_function
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
[]

[Functions]
  [left_function]
    type = ParsedOptimizationFunction
    expression = 'a + b*y'
    param_symbol_names = 'a b'
    param_vector_name = 'params_left/vals'
  []
  [right_function]
    type = ParsedOptimizationFunction
    expression = 'a'
    param_symbol_names = 'a'
    param_vector_name = 'params_right/vals'
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
  line_search = none
  solve_type = NEWTON
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[VectorPostprocessors]
  [vertical_1]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0.2 0.0 0'
    end_point = '0.2 2.0 0'
    num_points = 21
    sort_by = y
  []
  [vertical_2]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0.8 0.0 0'
    end_point = '0.8 2.0 0'
    num_points = 21
    sort_by = y
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
  []
  [params_left]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '0 0' # Dummy
  []
  [params_right]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '0' # Dummy
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'homogenous'
[]
