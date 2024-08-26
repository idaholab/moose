id = 1
frequencyHz = 1.0
omega = '${fparse 2*3.14159265359*frequencyHz}'

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
  skip_nl_system_check = true
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_forced_its = 1
  line_search = none
  nl_abs_tol = 1e-8
[]

[Outputs]
  csv = false
  console = false
  json = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 200
  xmax = 2
[]

[Variables]
  [ur]
  []
  [ui]
  []
  [uradj]
    initial_condition = 0
    solver_sys = adjoint
  []
  [uiadj]
    initial_condition = 0
    solver_sys = adjoint
  []
[]

[Kernels]
  [stiff]
    type = MatDiffusion
    variable = ur
    diffusivity = G
  []
  [mass]
    type = Reaction
    variable = ur
    rate = '${fparse -1.0*omega*omega}'
  []
  [stiffV]
    type = MatDiffusion
    variable = ui
    diffusivity = G
  []
  [massV]
    type = Reaction
    variable = ui
    rate = '${fparse -1.0*omega*omega}'
  []
[]

[BCs]
  [leftU]
    type = CoupledVarNeumannBC
    boundary = left
    variable = ur
    coef = 2.0
    v = ui
  []
  [leftV]
    type = CoupledVarNeumannBC
    boundary = left
    variable = ui
    coef = -2.0
    v = ur
  []
  [right]
    type = NeumannBC
    variable = ur
    boundary = right
    value = 1
  []
[]

[Materials]
  [G]
    type = GenericFunctionMaterial
    prop_names = G
    prop_values = G_function
  []
[]

[Functions]
  [G_function]
    type = NearestReporterCoordinatesFunction
    x_coord_name = parameters/coordx
    value_name = parameters/G
  []
[]

[Reporters]
  [parameters]
    type = ConstantReporter
    real_vector_names = 'coordx G'
    # 'True value when used to generate synthetic data'
    # real_vector_values = '5.0 15.0; 4.0 4.0'
    real_vector_values = '1.0; 4.0'
  []
[]

[DiracKernels]
  [misfit_ur]
    type = ReporterPointSource
    variable = uradj
    x_coord_name = measure_data_ur/measurement_xcoord
    y_coord_name = measure_data_ur/measurement_ycoord
    z_coord_name = measure_data_ur/measurement_zcoord
    value_name = measure_data_ur/misfit_values
  []
  [misfit_ui]
    type = ReporterPointSource
    variable = uiadj
    x_coord_name = measure_data_ui/measurement_xcoord
    y_coord_name = measure_data_ui/measurement_ycoord
    z_coord_name = measure_data_ui/measurement_zcoord
    value_name = measure_data_ui/misfit_values
  []
[]

[VectorPostprocessors]
  [gradient_from_real]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = uradj
    forward_variable = ur
    function = G_function
    execute_on = ADJOINT_TIMESTEP_END
    # Just to confirm this happens BEFORE the gradient calcutions
    execution_order_group = -1
  []
  [gradient_from_imag]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = uiadj
    forward_variable = ui
    function = G_function
    execute_on = ADJOINT_TIMESTEP_END
    # Just to confirm this happens BEFORE the gradient calcutions
    execution_order_group = -1
  []
[]

[Reporters]
  [measure_data_ur]
    type = OptimizationData
    objective_name = obj_val
    variable = 'ur'
    file_value = 'value'
    measurement_file = 'measurement/frequency_ur_${id}.csv'
  []
  [measure_data_ui]
    type = OptimizationData
    objective_name = obj_val
    variable = 'ui'
    file_value = 'value'
    measurement_file = 'measurement/frequency_ui_${id}.csv'
  []
  [gradient]
    type = ParsedVectorReporter
    name = gradient
    reporter_names = 'gradient_from_real/inner_product gradient_from_imag/inner_product'
    reporter_symbols = 'a b'
    expression = 'a+b'
    execute_on = ADJOINT_TIMESTEP_END
    execution_order_group = 0
  []
  [objective]
    type = ParsedScalarReporter
    name = objective
    reporter_names = 'measure_data_ur/obj_val measure_data_ui/obj_val'
    reporter_symbols = 'a b'
    expression = 'a+b'
    execute_on = ADJOINT_TIMESTEP_END
    # Just to confirm this happens after the gradient calcutions
    execution_order_group = 1
  []
[]
