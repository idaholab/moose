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
    type = ADFunctorElementalGradientAux
    functor = forwardT
    variable = dDdTgradT
    factor_matprop = 'dDdT'
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
    type = ADParsedMaterial
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
  [params]
    type = ConstantReporter
    real_vector_names = 'heat_source'
    real_vector_values = '0' # Dummy
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
    type = ElementOptimizationSourceFunctionInnerProduct
    function = volumetric_heat_func
    variable = adjointT
  []
[]

[Functions]
  [volumetric_heat_func]
    type = ParsedOptimizationFunction
    expression = q
    param_symbol_names = 'q'
    param_vector_name = 'params/heat_source'
  []
[]

[Outputs]
  console = false
[]
