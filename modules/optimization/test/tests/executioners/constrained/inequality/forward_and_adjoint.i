[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 20
    xmax = 1
    ymax = 1
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
    solver_sys = adjoint
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
  [dc_db]
    type = ParsedFunction
    expression = 'y'
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
[]

[Postprocessors]
  [sum]
    type = FunctionSideIntegral
    boundary = left
    function = left_function
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
    objective_name = objective_value
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'left'
    real_vector_values = '0 0' # Dummy
    execute_on = NONE
  []

[]

[Outputs]
  console = false
  exodus = false
  json = true
[]

#---------Inequality constraints------------#

[VectorPostprocessors]
  [gradient_c]
    type = VectorOfPostprocessors
    postprocessors = 'dc_da dc_db'
  []
  [ineq]
    type = VectorOfPostprocessors
    postprocessors = 'constraint'
  []
[]

[Postprocessors]
  [constraint]
    type = ParsedPostprocessor
    expression = '150 - sum' # 150 is the constraint we want to satisfy
    pp_names = sum
  []
  [dc_da]
    type = FunctionSideIntegral
    boundary = left
    function = -1
  []
  [dc_db]
    type = FunctionSideIntegral
    boundary = left
    function = '-y'
  []
[]
