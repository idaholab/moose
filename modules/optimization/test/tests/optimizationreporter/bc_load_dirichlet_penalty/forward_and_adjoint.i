# Same inverse problem as bc_load_dirichlet, but the controlled Dirichlet data is
# imposed weakly, so the gradient is penalty*integral(adjoint*dg/dp) on the boundary.
penalty_factor = 1e5

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
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
    type = FunctionPenaltyDirichletBC
    variable = temperature
    boundary = left
    function = left_function
    penalty = ${penalty_factor}
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
  [penalty_scaled_left_function] # parameter gradient of this is penalty*dg/dp
    type = ParsedOptimizationFunction
    expression = '${penalty_factor}*(a + b*y)'
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
    function = penalty_scaled_left_function
    boundary = left
    execute_on = ADJOINT_TIMESTEP_END
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
    real_vector_names = 'vals'
    real_vector_values = '0 0' # Dummy
  []
[]

[Preconditioning]
  [nl0]
    type = SMP
    nl_sys = 'nl0'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
  [adjoint]
    type = SMP
    nl_sys = 'adjoint'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  line_search = none
  # penalty imposition raises the condition number, so 1e-12 is not reachable here
  nl_rel_tol = 1e-10
  l_tol = 1e-12
[]

[Outputs]
  console = false
[]
