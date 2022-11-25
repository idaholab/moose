[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 20
    xmax = 1
    ymax = 2
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[BCs]
  [left]
    type = ConvectiveFluxFunction
    variable = temperature
    boundary = 'left'
    T_infinity = 100.0
    coefficient = function1
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = -100
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 500
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 600
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Functions]
  [function1]
    type = ParsedOptimizationFunction
    expression = 'a'
    param_symbol_names = 'a'
    param_vector_name = 'params/vals'
  []
[]

[VectorPostprocessors]
  [vertical]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0.1 0.0 0.0'
    end_point = '0.1 2.0 0.0'
    num_points = 21
    sort_by = id
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '0' # Dummy value
  []
[]

[Outputs]
  csv = true
  exodus = false
  console = false
  file_base = 'forward'
[]
