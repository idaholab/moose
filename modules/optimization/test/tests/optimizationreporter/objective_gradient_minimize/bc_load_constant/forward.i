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
    value = 200
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 100
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[VectorPostprocessors]
  [data_pt]
    type = PointValueSampler
    sort_by = id
    variable = temperature
    points = '0.2 0.2 0
              0.8 0.6 0
              0.2 1.4 0
              0.8 1.8 0'
  []
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'forward'
[]
