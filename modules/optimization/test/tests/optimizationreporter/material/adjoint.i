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
    type = ParsedFunction
    value = alpha
    vars = 'alpha'
    vals = 'p1'
  []
[]

[Materials]
  [thermalProp]
    type = GenericFunctionMaterial
    prop_names = 'thermal_conductivity'
    prop_values = 'thermo_conduct'
  []
  [thermalPropDeriv]
    type = GenericFunctionMaterial
    prop_names = 'thermal_conductivity_deriv'
    prop_values = '1'
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

[Postprocessors]
  [pp_adjoint_grad]
    type = MaterialGradientIntegral
    adjoint_var = 'adjointVar'
    forward_var = temperature_forward
    material_derivative = thermal_conductivity_deriv
  []
  [p1]
    type = ConstantValuePostprocessor
    value = 10
    execute_on = 'initial linear'
  []
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[VectorPostprocessors]
  [adjoint_grad]
    type = VectorOfPostprocessors
    postprocessors = 'pp_adjoint_grad'
  []
[]

[Outputs]
  console = false
  file_base = 'adjoint'
[]
