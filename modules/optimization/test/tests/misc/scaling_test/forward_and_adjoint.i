[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    ymin = 0
    xmin = 0
    xmax = 10
    ymax = 10
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = FALSE
[]

[Variables]
  [T_real]
    initial_condition = 1e-8
    scaling = 10
  []
  [T_imag]
    initial_condition = 1e-8
  []
  [T_real_adj]
    solver_sys = adjoint
  []
  [T_imag_adj]
    solver_sys = adjoint
  []
[]

[Kernels]
  [heat_conduction_real]
    type = MatDiffusion
    variable = T_real
    diffusivity = k
  []
  [heat_source_real]
    type = MatCoupledForce
    variable = T_real
    v = T_imag
    material_properties = 'force_mat'
  []

  [heat_conduction_imag]
    type = MatDiffusion
    variable = T_imag
    diffusivity = k
  []
  [heat_source_imag]
    type = MatCoupledForce
    variable = T_imag
    v = T_real
    material_properties = 'force_mat'
    coef = -1
  []
[]

[Materials]
  [k_mat]
    type = GenericFunctionMaterial
    prop_names = 'k'
    prop_values = 'kappa_func'
  []
  [mats]
    type = GenericConstantMaterial
    prop_names = 'rho omega cp '
    prop_values = '1.0 1.0 1.0 '
  []
  [force_mat]
    type = ParsedMaterial
    property_name = force_mat
    expression = 'rho * omega * cp'
    material_property_names = 'rho omega cp'
  []
  [phase]
    type = ADParsedMaterial
    coupled_variables = 'T_real T_imag'
    expression = 'atan2(T_imag, T_real)'
    property_name = phase
  []
[]

[Functions]
  [gauss]
    type = ParsedFunction
    expression = 'exp(-2.0 *(x^2 + y^2 + z^2)/(beam_radii^2))'
    symbol_names = 'beam_radii'
    symbol_values = '0.1'
  []
  [kappa_func]
    type = ParsedOptimizationFunction
    expression = 'k '
    param_symbol_names = 'k '
    param_vector_name = 'params/k'
  []
[]

[BCs]
  [real_top]
    type = FunctionNeumannBC
    variable = T_real
    boundary = top
    function = 'exp((-2.0 *(x)^2)/0.1)'
  []
[]

[DiracKernels]
  [misfit_real]
    type = ReporterPointSource
    variable = T_real_adj
    x_coord_name = measure_data_real/measurement_xcoord
    y_coord_name = measure_data_real/measurement_ycoord
    z_coord_name = measure_data_real/measurement_zcoord
    value_name = measure_data_real/misfit_values
  []
  [misfit_imag]
    type = ReporterPointSource
    variable = T_imag_adj
    x_coord_name = measure_data_imag/measurement_xcoord
    y_coord_name = measure_data_imag/measurement_ycoord
    z_coord_name = measure_data_imag/measurement_zcoord
    value_name = measure_data_imag/misfit_values
  []
[]

[AuxVariables]
  [phase]
  []
[]

[AuxKernels]
  [phase]
    type = ParsedAux
    variable = phase
    coupled_variables = 'T_imag T_real'
    expression = 'atan2(T_imag, T_real)'
    execute_on = 'TIMESTEP_END'
  []
[]

[Reporters]
  [measure_data_real]
    type = OptimizationData
    variable = T_real
    objective_name = objective_value
    measurement_values = '0.10391 -0.0064'
    measurement_points = '0.55 10 0
                          3.55 10 0'
  []
  [measure_data_imag]
    type = OptimizationData
    objective_name = objective_value
    variable = T_imag
    measurement_values = '-0.08234 -0.00181'
    measurement_points = '0.55 10 0
                          3.55 10 0'
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'k'
    real_vector_values = '2' # Dummy value
  []
  [gradient]
    type = ParsedVectorReporter
    name = inner
    reporter_names = 'gradient_real/inner_product gradient_imag/inner_product'
    reporter_symbols = 'a b'
    expression = 'a+b'
    execute_on = ADJOINT_TIMESTEP_END
    execution_order_group = 1
  []
  [obj]
    type = ParsedScalarReporter
    name = value
    reporter_names = 'measure_data_real/objective_value measure_data_imag/objective_value'
    reporter_symbols = 'a b'
    expression = 'a+b'
    execute_on = ADJOINT_TIMESTEP_END
    execution_order_group = 1
  []
[]

[VectorPostprocessors]
  [gradient_real]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = T_real_adj
    forward_variable = T_real
    function = kappa_func
    execute_on = ADJOINT_TIMESTEP_END
  []
  [gradient_imag]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = T_imag_adj
    forward_variable = T_imag
    function = kappa_func
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-10

  automatic_scaling = false

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = l2
  verbose = true
[]

[Outputs]
  console = true
  execute_on = FINAL
[]
