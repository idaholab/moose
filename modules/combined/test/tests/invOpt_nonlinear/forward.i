[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = none
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Mesh]
[]

[Variables]
  [forwardT]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    thermal_conductivity = 'conductivity'
    variable = forwardT
  []
  [heat_source]
    type = ADMatHeatSource
    material_property = 'volumetric_heat'
    variable = forwardT
  []
[]

[Materials]
  [NonlinearConductivity]
    type = ADParsedMaterial
    f_name = 'conductivity'
    function = '10+500*forwardT'
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
    type = ParsedOptimizationFunction
    expression = q
    param_symbol_names = 'q'
    param_vector_name = 'params/heat_source'
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = forwardT
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = forwardT
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = forwardT
    boundary = bottom
    value = 2
  []
  [top]
    type = DirichletBC
    variable = forwardT
    boundary = top
    value = 1
  []
[]

[Reporters]
  [measurement_locations]
    type = OptimizationData
    variable = forwardT
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'heat_source'
    real_vector_values = '0' # Dummy
  []
[]

[Outputs]
  console = false
[]
