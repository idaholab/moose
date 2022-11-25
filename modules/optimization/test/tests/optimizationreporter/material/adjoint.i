[Mesh]
[]

[Variables]
  [adjointVar]
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = adjointVar
    diffusivity = thermal_conductivity
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = adjointVar
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
  []
[]

[Reporters]
  [misfit]
    type = OptimizationData
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'p1'
    real_vector_values = '0' # Dummy value
  []
[]

[AuxVariables]
  [temperature_forward]
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = adjointVar
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = adjointVar
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = adjointVar
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = adjointVar
    boundary = top
    value = 0
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
  [thermalProp]
    type = GenericFunctionMaterial
    prop_names = 'thermal_conductivity'
    prop_values = 'thermo_conduct'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[VectorPostprocessors]
  [adjoint_grad]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = adjointVar
    forward_variable = temperature_forward
    function = thermo_conduct
  []
[]

[Outputs]
  console = false
  file_base = 'adjoint'
[]
