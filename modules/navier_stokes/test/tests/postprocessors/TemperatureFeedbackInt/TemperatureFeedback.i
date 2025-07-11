[Mesh]
  coord_type = 'RZ'
  rz_coord_axis = Y
    #file = 'initial.e'
  file = 'ns_initial.e'
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
  [T]
    initial_condition = 1
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


[MultiApps]
  [Reader]
    type = FullSolveMultiApp
    input_files = "Reader.i"
    execute_on= INITIAL
  []
[]


[Transfers]
   [pull_flux_inital]
    type = MultiAppShapeEvaluationTransfer
    from_multi_app = Reader 
    source_variable =flux 
    variable = power
    execute_on= INITIAL
  [] 
[]

[Postprocessors]
  [Temp_feedback]
      type = TemperatureFeedbackInt
      variable = T
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
[]
