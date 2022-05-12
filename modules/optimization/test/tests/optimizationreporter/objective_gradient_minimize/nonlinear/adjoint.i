[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = none
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  nl_max_its = 100
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]


[Mesh]
[]
[Variables]
  [adjointT]
  []
[]
[AuxVariables]
  [forwardT]
  []
  [dDdTgradT]
    order  = CONSTANT
    family = MONOMIAL_VEC
  []
[]


[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    thermal_conductivity = 'linearized_conductivity'
    variable = adjointT
  []
  [advection]
    type = LevelSetAdvection
    velocity = dDdTgradT
    variable = adjointT
  []
[]
[AuxKernels]
  [dDdTgradT]
    type = MaterialScaledGradientVector
    gradient_variable = forwardT
    variable = dDdTgradT
    material_scaling = 'dDdT'
  []
[]
[Materials]
  [LinearizedConductivity]
    type = ADParsedMaterial
    f_name = 'linearized_conductivity'
    function = '10+500*forwardT'
    args = 'forwardT'
  []
  [dDdT]
    type = ParsedMaterial
    f_name = 'dDdT'
    function = '500'
    args = 'forwardT'
  []
[]


[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = adjointT
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name   = misfit/misfit_values
  []
[]
[Reporters]
  [misfit]
    type = OptimizationData
  []
[]


[BCs]
  [left]
    type = NeumannBC
    variable = adjointT
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = adjointT
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = adjointT
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = adjointT
    boundary = top
    value = 0
  []
[]


[VectorPostprocessors]
  [gradient_vpp]
    type = VectorOfPostprocessors
    postprocessors = 'heatSourceGradient'
  []
[]
[Postprocessors]
  [heatSourceGradient]
    type = VariableFunctionElementIntegral
    function = volumetric_heat_func_deriv
    variable = adjointT
  []
[]
[Functions]
  [volumetric_heat_func_deriv]
    type = ParsedFunction
    value = dq
    vars = 'dq'
    vals = 1
  []
[]


[Outputs]
  console = false
[]
