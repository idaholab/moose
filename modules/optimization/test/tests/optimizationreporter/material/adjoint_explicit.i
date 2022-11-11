[Mesh]
[]

[Variables]
  [adjoint_var]
  []
[]
[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = adjoint_var
    diffusivity = thermal_conductivity
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = adjoint_var
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
  [grad_Tx]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Ty]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Tz]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Tfx]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Tfy]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Tfz]
    order = CONSTANT
    family = MONOMIAL
  []
  [negative_gradient]
    order = CONSTANT
    family = MONOMIAL
  []
[]
[AuxKernels]
  [grad_Tx]
    type = VariableGradientComponent
    component = x
    variable = grad_Tx
    gradient_variable = adjoint_var
  []
  [grad_Ty]
    type = VariableGradientComponent
    component = y
    variable = grad_Ty
    gradient_variable = adjoint_var
  []
  [grad_Tz]
    type = VariableGradientComponent
    component = z
    variable = grad_Tz
    gradient_variable = adjoint_var
  []
  [grad_Tfx]
    type = VariableGradientComponent
    component = x
    variable = grad_Tfx
    gradient_variable = temperature_forward
  []
  [grad_Tfy]
    type = VariableGradientComponent
    component = y
    variable = grad_Tfy
    gradient_variable = temperature_forward
  []
  [grad_Tfz]
    type = VariableGradientComponent
    component = z
    variable = grad_Tfz
    gradient_variable = temperature_forward
  []
  [negative_gradient]
    type = ParsedAux
    variable = negative_gradient
    args = 'grad_Tx grad_Ty grad_Tz grad_Tfx grad_Tfy grad_Tfz'
    function = '-(grad_Tx*grad_Tfx+grad_Ty*grad_Tfy+grad_Tz*grad_Tfz)'
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = adjoint_var
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = adjoint_var
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = adjoint_var
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = adjoint_var
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
  solve_type = NEWTON
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[Postprocessors]
  [pp_adjoint_grad_parsedFunc]
    type = ElementIntegralVariablePostprocessor
    variable = negative_gradient
    execute_on = 'initial linear'
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
    postprocessors = 'pp_adjoint_grad_parsedFunc'
  []
[]

[Outputs]
  console = false
  file_base = 'adjoint'
[]
