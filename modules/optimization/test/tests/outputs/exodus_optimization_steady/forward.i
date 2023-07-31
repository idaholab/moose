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
  [heat_source]
    type = BodyForce
    value = 1000
    variable = temperature
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 0
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
  [thermo_conduct]
    type = ParsedOptimizationFunction
    expression = 'alpha'
    param_symbol_names = 'alpha'
    param_vector_name = 'params/p1'
  []
[]

[Materials]
  [steel]
    type = GenericFunctionMaterial
    prop_names = 'thermal_conductivity'
    prop_values = 'thermo_conduct'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'p1'
    real_vector_values = '0' # Dummy value
  []
[]

[Outputs]
  console = false
  file_base = 'forward'
[]
