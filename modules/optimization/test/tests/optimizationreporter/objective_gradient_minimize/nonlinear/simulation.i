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
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
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
    type = ParsedFunction
    value = q
    vars = 'q'
    vals = 'heat_source_pp'
  []
[]
[Postprocessors]
  [heat_source_pp]
    type = ConstantValuePostprocessor
    value = 333
    execute_on = 'LINEAR'
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
  []
[]
[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[VectorPostprocessors]
  [data_pt]
    type = PointValueSampler
    variable = forwardT
    points = '0.2 0.2 0
              0.8 0.6 0
              0.2 1.4 0
              0.8 1.8 0'
    sort_by = id
  []
[]

[Outputs]
  csv = true
[]
