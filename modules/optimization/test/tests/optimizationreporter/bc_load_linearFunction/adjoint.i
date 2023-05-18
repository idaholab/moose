[Mesh]
[]

[Variables]
  [adjoint_T]
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = adjoint_T
    diffusivity = thermal_conductivity
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = adjoint_T
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
    weight_name = misfit/weight
  []
[]

[Reporters]
  [misfit]
    type = OptimizationData
    variable_weight_names = 'weight'
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

[BCs]
  [left]
    type = NeumannBC
    variable = adjoint_T
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = adjoint_T
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = adjoint_T
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = adjoint_T
    boundary = top
    value = 0
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

[VectorPostprocessors]
  [grad_bc_left]
    type = SideOptimizationNeumannFunctionInnerProduct
    variable = adjoint_T
    function = left_function
    boundary = left
  []
  [grad_bc_right]
    type = SideOptimizationNeumannFunctionInnerProduct
    variable = adjoint_T
    function = right_function
    boundary = right
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'adjoint'
[]
