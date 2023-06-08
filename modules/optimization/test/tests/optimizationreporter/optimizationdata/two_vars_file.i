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
    order = CONSTANT
    family = MONOMIAL
  []
  [disp_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [T]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [aux_disp_x]
    type = ParsedAux
    variable = disp_x
    use_xyzt = true
    expression = 'if(x<0.5,2,4)'
  []
  [aux_disp_y]
    type = ParsedAux
    variable = disp_y
    use_xyzt = true
    expression = 'if(y<0.5,10,50)'
  []
  [aux_T]
    type = ParsedAux
    variable = T
    use_xyzt = true
    expression = 'x'
  []
[]


[Reporters]
  [measure_data]
    type = OptimizationData
    measurement_file = 'measurementData.csv'
    file_value = 'measured_value'
    variable = 'disp_x disp_y T'
    file_variable_weights = 'weight_u v_weight wT'
    variable_weight_names = 'weight_u v_weight wT'
  []
[]

[BCs]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  exodus=true
[]
[Debug]
  show_reporters = false
[]
