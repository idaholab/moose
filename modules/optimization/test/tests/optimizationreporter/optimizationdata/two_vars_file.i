[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Problem]
  solve=false
[]

[AuxVariables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [aux_disp_x]
    type = ParsedAux
    variable = disp_x
    use_xyzt = true
    expression = '2*x'
  []
  [aux_disp_y]
    type = ParsedAux
    variable = disp_y
    use_xyzt = true
    expression = 'y'
  []
[]


[Reporters]
  [measure_data]
    type = OptimizationData
    measurement_file = 'measurementData.csv'
    file_value = 'measured_value'
    variable = 'disp_x disp_y'
    file_variable_weights = 'weight_u v_weight'
  []
[]

[BCs]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
[Debug]
  show_reporters = false
[]
