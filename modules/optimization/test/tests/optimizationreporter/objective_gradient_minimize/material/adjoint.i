[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
[]

[Variables]
  [temperature]
  []
[]
[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  # [heat_source]
  #   type = ADMatHeatSource
  #   material_property = volumetric_heat
  #   variable = temperature
  # []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = temperature
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
  []
[]

[Reporters]
  [misfit]
    type=OptimizationData
  []
[]
[AuxVariables]
  [forwardAdjoint]
  []
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
  [gradient]
    order = CONSTANT
    family = MONOMIAL
  []
[]
[AuxKernels]
  [grad_Tx]
    type = VariableGradientComponent
    component = x
    variable = grad_Tx
    gradient_variable = temperature
  []
  [grad_Ty]
    type = VariableGradientComponent
    component = y
    variable = grad_Ty
    gradient_variable = temperature
  []
  [grad_Tz]
    type = VariableGradientComponent
    component = z
    variable = grad_Tz
    gradient_variable = temperature
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
  [gradient]
    type = ParsedAux
    variable = gradient
    args = 'grad_Tx grad_Ty grad_Tz grad_Tfx grad_Tfy grad_Tfz'
    function = 'grad_Tx*grad_Tfx+grad_Ty*grad_Tfy+grad_Tz*grad_Tfz'
  []
  [forwardAdjoint]
    type = ParsedAux
    variable = forwardAdjoint
    args = 'temperature_forward temperature'
    function = 'temperature_forward*temperature'
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
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
  [heat_source]
    type = ParsedFunction
    # value = 100*cos(2*pi/2*(x+1))
    value = 1000
  []
[]

#fixme Lynn parsedMaterial and DerivativeParsedMaterial
[Materials]
  [steel]
    type = GenericFunctionMaterial
    prop_names = 'thermal_conductivity'
    prop_values = 'thermo_conduct'
  []
  [steel_deriv]
    type = GenericFunctionMaterial
    prop_names = 'thermal_conductivity_deriv'
    prop_values = '1'
  []
  # [volumetric_heat]
  #   type = ADGenericFunctionMaterial
  #   prop_names = 'volumetric_heat'
  #   prop_values = 'heat_source'
  # []
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
    # integral of load function gradient w.r.t parameter
    type = DiffusionVariableIntegral
    variable1 = temperature
    variable2 = temperature_forward
    material_derivative = thermal_conductivity_deriv
    execute_on = 'initial linear'
  []
  [pp_adjoint_grad_parsedFunc]
    type = ElementIntegralVariablePostprocessor
    variable = gradient
    execute_on = 'initial linear'
  []
  [pp_forwardAdjoint]
    type = ElementIntegralVariablePostprocessor
    variable = forwardAdjoint
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
    postprocessors = 'pp_adjoint_grad'
  []
[]

[Outputs]
  console = false
  exodus = false
  csv = false
  file_base = 'adjoint'
  execute_on = NONLINEAR
[]
