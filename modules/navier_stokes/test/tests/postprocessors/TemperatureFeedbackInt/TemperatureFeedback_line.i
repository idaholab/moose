[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 100
  bias_x = 1.1
[]

[Variables]
  [C]
  []
[]


[AuxVariables]
  [power]
  []
  [flux]
  []
  [T1]
    initial_condition = 1
  []
  [T2]
  []
  [T3]
  []
  [T_ref]
    initial_condition = 0
  []
[]

[Kernels]
  [C_time]
    type = NullKernel
    variable = C
  []
[]

[AuxKernels]
    [power_scale]
        type = NormalizationAux 
        variable = flux
        source_variable = power
        normalization = power_int
    []
[]

[ICs]
  [T3_initial]
    type = FunctionIC
    variable = 'T3'
    function = initial_T3
  []
  [T2_initial]
    type = FunctionIC
    variable = 'T2'
    function = initial_T2 
  []
  [power_initial]
    type = FunctionIC
    variable = 'power'
    function = initial_power 
  []
[]

[Functions]
  [initial_T3]
    type = PiecewiseConstant
    axis = x
    xy_data = '0 0
               0.5 1'
    direction = 'left'
  []
  [initial_T2]
    type = PiecewiseConstant
    axis = x
    xy_data = '0 1
               0.5 0'
    direction = 'left_inclusive'
  []
  [initial_power]
    type = ParsedFunction
    expression = 'sin(pi*x)'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 3
[]

[Outputs]
  exodus = true
  [outfile]
    type = CSV
    delimiter = ' '
  []
[]


[Postprocessors]
  [Temp_feedback_3]
      type = TemperatureFeedbackInt
      variable = T3
      flux = flux
      T_ref = T_ref
      total_rho = -4
  []
  [Temp_feedback_2]
      type = TemperatureFeedbackInt
      variable = T2
      flux = flux
      T_ref = T_ref
      total_rho = -4
  []
  [Temp_feedback_1]
      type = TemperatureFeedbackInt
      variable = T1
      flux = flux
      T_ref = T_ref
      total_rho = -4
  []
 [flux_sum]
     type = NodalSum 
     variable = flux
 []
 [flux_int]
    type = ElementIntegralVariablePostprocessor
     variable = flux
 []
 [power_int]
    type = ElementIntegralVariablePostprocessor
     variable = power
 []
 [T2_int]
    type = ElementIntegralVariablePostprocessor
     variable = T2
 []
 [T3_int]
    type = ElementIntegralVariablePostprocessor
     variable = T3
 []
[]
