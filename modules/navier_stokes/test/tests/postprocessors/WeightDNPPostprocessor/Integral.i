[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1.0
    nx = 200
  []
[]

[Variables]
  [C]
      family = MONOMIAL
      order = CONSTANT
      fv = true
  []
  [C1]
      family = MONOMIAL
      order = CONSTANT
      fv = true
  []
[]


[AuxVariables]
  [V1]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [V2]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [V3]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[ICs]
  [C_ic]
    type = FunctionIC
    variable = 'C'
    function = parsed_function_1
  []
  [C1_ic]
    type = FunctionIC
    variable = 'C1'
    function = parsed_function_3
  []
  # parallel
  [V1_ic]
    type = FunctionIC
    variable = 'V1'
    function = parsed_function_1
  []
  #orthorgonal
  [V2_ic]
    type = FunctionIC
    variable = 'V2'
    function = parsed_function_2
  []
  #mix
  [V3_ic]
    type = FunctionIC
    variable = 'V3'
    function = parsed_function_3
  []
[]

[Functions]
  [parsed_function_1]
    type = ParsedFunction
    expression = 'x*x'
  []
  [parsed_function_2]
    type = ParsedFunction
    expression = 'x'
  []
  [parsed_function_3]
    type = ParsedFunction
    expression = 'exp(x)'
  []
[]

[FVKernels]
  [C_time]
    type = FVTimeKernel
    variable = C
  []
  [C1_time]
    type = FVTimeKernel
    variable = C1
  []
[]

[Executioner]
  type = Transient
  dt = 0.001
  start_time = 0# ${fparse -dt}
  end_time = 0.002
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-4
  l_max_its = 200
[]

[Outputs]
  exodus = true
  [outfile]
    type = CSV
    delimiter = ' '
  []
[]



[Postprocessors]
  [C_int]
    #type = ElementExtremeValue
    type = ElementIntegralVariablePostprocessor
    #type = NodalSum
    variable = C
    execute_on = 'initial timestep_end'
 []
 [B]
     type = ElementL2Norm
     variable = C
    execute_on = 'initial timestep_end'
 []
 [A1]
     type = WeightDNPPostprocessor
     variable = V1
     other_variable = C
     Norm = B
     lambda =1 
    execute_on = 'timestep_end'
 []
 [A2]
     type = WeightDNPPostprocessor
     variable = V2
     other_variable = C
     Norm = B
     lambda =1 
    execute_on = ' timestep_end'
 []
 [A3]
     type = WeightDNPPostprocessor
     variable = V3
     other_variable = C
     Norm = B
     lambda =1 
    execute_on = 'timestep_end'
 []
[]
