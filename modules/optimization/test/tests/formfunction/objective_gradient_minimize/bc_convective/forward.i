[Mesh]
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[BCs]
  [left]
    type = ConvectiveFluxFunction
    variable = temperature
    boundary = 'left'
    T_infinity = 100.0
    coefficient = function1
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = -100
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 500
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 600
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
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

[Functions]
  [function1]
    type = ParsedFunction
    value = a*1.0
    vars = 'a'
    vals = 'p1'
  []
[]

[Postprocessors]
  [p1]
    type = ConstantValuePostprocessor
    value = 1
    execute_on = LINEAR
  []
[]

[VectorPostprocessors]
  [./vertical]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0.1 0.0 0.0'
    end_point =   '0.1 2.0 0.0'
    num_points = 21
    sort_by = id
  [../]
  [data_pt]
    type = MeasuredDataPointSampler
    variable = temperature
    points = '0.1	0	0
0.1	0.1	0
0.1	0.2	0
0.1	0.3	0
0.1	0.4	0
0.1	0.5	0
0.1	0.6	0
0.1	0.7	0
0.1	0.8	0
0.1	0.9	0
0.1	1	0
0.1	1.1	0
0.1	1.2	0
0.1	1.3	0
0.1	1.4	0
0.1	1.5	0
0.1	1.6	0
0.1	1.7	0
0.1	1.8	0
0.1	1.9	0
0.1	2	0'
    measured_values = '500
472.9398111
450.8117197
434.9560747
423.3061045
414.9454912
409.3219399
406.1027006
405.0865428
406.1604905
409.2772668
414.4449772
421.7253934
431.2401042
443.1862012
457.8664824
475.7450186
497.5582912
524.4966003
559.1876637
600'
  []
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  csv=true
  exodus = true
  file_base = 'forward'
[]
