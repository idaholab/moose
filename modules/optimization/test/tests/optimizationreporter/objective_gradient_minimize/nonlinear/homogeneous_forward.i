[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = none
  #nl_forced_its = 1
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]


[Mesh]
[]
[Variables]
  [T]
  []
[]
[AuxVariables]
  [forwardT]
  []
  [_dDdTgradT]
    order  = CONSTANT
    family = MONOMIAL_VEC
  []
[]


[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    thermal_conductivity = 'linearized_conductivity'
    variable = T
  []
  [heat_source]
    type = ADMatHeatSource
    material_property = 'volumetric_heat'
    variable = T
  []
  [advection]
    type = ConservativeAdvection
    velocity = _dDdTgradT
    variable = T
    upwinding_type = full  #Full upwinding gives somewhat better results
  []
[]
[AuxKernels]
  [_dDdTgradT]
    type = MaterialScaledGradientVector
    gradient_variable = forwardT
    variable = _dDdTgradT
    material_scaling = '_dDdT'
  []
[]
[Materials]
  [LinearizedConductivity]
    type = ADParsedMaterial
    f_name = 'linearized_conductivity'
    function = '10+500*forwardT'
    args = 'forwardT'
  []
  [_dDdT]
    type = ParsedMaterial
    f_name = '_dDdT' # "_" represents negation
    function = '-500'
    args = 'forwardT'
  []
  [volumetric_heat]
    type = ADGenericFunctionMaterial
    prop_names = 'volumetric_heat'
    prop_values = 'volumetric_heat_func'
  []
[]
[Functions]
  [volumetric_heat_func]
    type = ParsedFunction
    value = q
    vars = 'q'
    vals = 'heat_source_pp'
  []
[]
[Postprocessors]
  [heat_source_pp]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = 'LINEAR'
  []
[]


[BCs]
  [left]
    type = NeumannBC
    variable = T
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = T
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = T
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = T
    boundary = top
    value = 0
  []
[]


[VectorPostprocessors]
  [data_pt]
    type = VppPointValueSampler
    variable = T
    reporter_name = measurement_locations
    outputs = none
  []
[]
[Reporters]
  [measurement_locations]
    type = OptimizationData
  []
[]
[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  console = false
[]
