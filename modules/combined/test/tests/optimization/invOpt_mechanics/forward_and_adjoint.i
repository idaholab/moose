# DO NOT CHANGE THIS TEST
# this test is documented as an example in forceInv_NeumannBC.md
# if this test is changed, the figures will need to be updated.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
  xmin = 0.0
  xmax = 5.0
  ymin = 0.0
  ymax = 1.0
  use_displaced_mesh = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      displacements = 'disp_x disp_y'
      [all]
        displacements = 'disp_x disp_y'
        strain = SMALL
      []
    []
  []
[]

[BCs]
  [left_ux]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [left_uy]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [right_fy]
    type = FunctionNeumannBC
    variable = disp_y
    boundary = right
    function = right_fy_func
  []
  [right_fx]
    type = FunctionNeumannBC
    variable = disp_x
    boundary = right
    function = right_fx_func
  []
[]

[Functions]
  [right_fy_func]
    type = ParsedOptimizationFunction
    expression = 'val_y'
    param_symbol_names = 'val_x val_y'
    param_vector_name = 'params/right_values'
  []
  [right_fx_func]
    type = ParsedOptimizationFunction
    expression = 'val_x'
    param_symbol_names = 'val_x val_y'
    param_vector_name = 'params/right_values'
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10e3
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[VectorPostprocessors]
  [point_sample]
    type = PointValueSampler
    variable = 'disp_x disp_y'
    points = '5.0 1.0 0'
    sort_by = x
  []
[]

[Reporters]
  [measure_data_x]
    type = OptimizationData
    objective_name = objective_value
    variable = disp_x
    measurement_points = '5.0 1.0 0.0'
    measurement_values = '-13.00873469088'
  []
  [measure_data_y]
    type = OptimizationData
    objective_name = objective_value
    variable = disp_y
    measurement_points = '5.0 1.0 0.0'
    measurement_values = '85.008027719915'
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'right_values'
    real_vector_values = '-1300 2100 ' # True Values
  []
  [combined]
    type = ParsedVectorReporter
    name = gradient
    reporter_names = 'adjoint_pt_x/inner_product adjoint_pt_y/inner_product'
    reporter_symbols = 'a b'
    expression = 'a+b'
    execute_on = ADJOINT_TIMESTEP_END
    # Just to confirm this happens after the gradient calcutions
    execution_order_group = 1
  []
  [obj]
    type = ParsedScalarReporter
    name = obj_val
    reporter_names = 'measure_data_x/objective_value
    measure_data_y/objective_value'
    reporter_symbols = 'a b'
    expression = 'a+b'
    execute_on = ADJOINT_TIMESTEP_END
    # Just to confirm this happens after the gradient calcutions
    execution_order_group = 1
  []
[]

[Outputs]
  csv = false
  console = false
  exodus = false
  file_base = 'forward'
  execute_on = 'FINAL'
  json = false
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
  skip_nl_system_check = true
[]

[Variables]
  [adjoint_x]
    initial_condition = 0
    solver_sys = adjoint
    outputs = none
  []
  [adjoint_y]
    initial_condition = 0
    solver_sys = adjoint
    outputs = none
  []
[]

[DiracKernels]
  [pt_x]
    type = ReporterPointSource
    variable = adjoint_x
    x_coord_name = measure_data_x/measurement_xcoord
    y_coord_name = measure_data_x/measurement_ycoord
    z_coord_name = measure_data_x/measurement_zcoord
    value_name = measure_data_x/misfit_values
  []
  [pt_y]
    type = ReporterPointSource
    variable = adjoint_y
    x_coord_name = measure_data_y/measurement_xcoord
    y_coord_name = measure_data_y/measurement_ycoord
    z_coord_name = measure_data_y/measurement_zcoord
    value_name = measure_data_y/misfit_values
  []
[]

[VectorPostprocessors]
  [adjoint_pt_x]
    type = SideOptimizationNeumannFunctionInnerProduct
    variable = adjoint_x
    function = right_fx_func
    boundary = right
    execute_on = ADJOINT_TIMESTEP_END
  []
  [adjoint_pt_y]
    type = SideOptimizationNeumannFunctionInnerProduct
    variable = adjoint_y
    function = right_fy_func
    boundary = right
    execute_on = ADJOINT_TIMESTEP_END
  []
[]
