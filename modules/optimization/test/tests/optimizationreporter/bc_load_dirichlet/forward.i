# Exact solution is 'temperature = 200 - 50*x + 25*y' at the true parameters 'a = 200, b = 25'.

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
    type = FunctionDirichletBC
    variable = temperature
    boundary = left
    function = left_function
  []
  [right]
    type = FunctionDirichletBC
    variable = temperature
    boundary = right
    function = right_function
  []
  [bottom]
    type = NeumannBC
    variable = temperature
    boundary = bottom
    value = -125
  []
  [top]
    type = NeumannBC
    variable = temperature
    boundary = top
    value = 125
  []
[]

[Functions]
  [left_function] # controlled Dirichlet data on the inverted boundary
    type = ParsedOptimizationFunction
    expression = 'a + b*y'
    param_symbol_names = 'a b'
    param_vector_name = 'params/vals'
  []
  [right_function]
    type = ParsedFunction
    expression = '150 + 25*y'
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
  solve_type = NEWTON
  line_search = none
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
    objective_name = objective_value
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '0 0' # Dummy
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'forward'
[]
