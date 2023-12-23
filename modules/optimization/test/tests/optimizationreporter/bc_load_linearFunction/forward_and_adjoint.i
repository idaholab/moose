[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 20
    xmax = 1
    ymax = 2
    bias_x = 1.1
    bias_y = 1.1
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [temperature]
  []
  [temperature_adjoint]
    nl_sys = adjoint
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
  [left_function]
    type = ParsedOptimizationFunction
    expression = 'a + b*y'
    param_symbol_names = 'a b'
    param_vector_name = 'params/left'
  []
  [right_function]
    type = ParsedOptimizationFunction
    expression = 'a'
    param_symbol_names = 'a'
    param_vector_name = 'params/right'
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
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  line_search = none
  nl_rel_tol = 1e-12
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = temperature_adjoint
    x_coord_name = measure_data/measurement_xcoord
    y_coord_name = measure_data/measurement_ycoord
    z_coord_name = measure_data/measurement_zcoord
    value_name = measure_data/misfit_values
    weight_name = measure_data/weight
  []
[]

[VectorPostprocessors]
  [grad_bc_left]
    type = SideOptimizationNeumannFunctionInnerProduct
    variable = temperature_adjoint
    function = left_function
    boundary = left
    execute_on = ADJOINT_TIMESTEP_END
  []
  [grad_bc_right]
    type = SideOptimizationNeumannFunctionInnerProduct
    variable = temperature_adjoint
    function = right_function
    boundary = right
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
    measurement_file = 'measurementData.csv'
    file_xcoord = 'coordx'
    file_ycoord ='y'
    file_zcoord = 'z'
    file_value = 'weightedMeasurement'
    file_variable_weights = 'weight'
    variable_weight_names = 'weight'
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'left right'
    real_vector_values = '0 0; 0' # Dummy
  []
  [vector_sqsum]
    type = ParsedVectorRealReductionReporter
    name = sqsum
    reporter_name= 'measure_data/misfit_values'
    initial_value = 0
    expression = 'reduction_value+indexed_value*indexed_value'
    outputs=none
  []
  [obj_sum]
    type = ParsedScalarReporter
    name = value
    reporter_names = 'vector_sqsum/sqsum'
    reporter_symbols = 'a'
    expression = '0.5*a'
  []
[]

[Outputs]
  console = false
  exodus = true
[]
