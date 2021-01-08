[Mesh]
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
[]

[BCs]
  [left]
    type = FunctionNeumannBC
    variable = temperature
    boundary = left
    function = sin_function
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 100
  []
  [bottom]
    type = FunctionNeumannBC
    variable = temperature
    boundary = bottom
    function = sin_function
  []
  [top]
    type = FunctionNeumannBC
    variable = temperature
    boundary = top
    function = sin_function
  []
[]

[Functions]
  [sin_function]
    type = ParsedFunction
    value = a*sin(2*pi*b*(y+c))+d
    vars = 'a b c d'
    vals = '500 0.5 p1 p2'
  []
[]

[Materials]
  [steel]
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [data_pt]
    type = MeasuredDataPointSampler
    variable = temperature
    points = '0.2 0.2 0
              0.8 0.6 0
              0.2 1.4 0
              0.8 1.8 0'
    measured_values = '228 128 214 151'
  []
[]

[Postprocessors]
  [p1]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p2]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  exodus = true
  file_base = 'forward'
[]
