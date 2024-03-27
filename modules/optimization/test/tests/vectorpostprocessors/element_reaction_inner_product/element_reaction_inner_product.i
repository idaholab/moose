[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[AuxVariables]
  [u]
  []
  [v]
    initial_condition = 1
  []
  [dp1]
  []
  [dp2]
  []
[]

p1 = 3.14
p2 = 2.72
[Reporters]
  [params]
    type = ConstantReporter
    real_vector_names = 'p'
    real_vector_values = '${p1} ${p2}'
  []
[]

[Functions]
  [p_fun]
    type = ParsedOptimizationFunction
    expression = 'p_1*x + p_2*p_2'
    param_symbol_names = 'p_1 p_2'
    param_vector_name = params/p
  []
  [u_fun]
    type = ParsedFunction
    expression = 'x'
  []
  [dp1_fun]
    type = ParsedFunction
    expression = 'x'
    symbol_names = 'p_1 p_2'
    symbol_values = '${p1} ${p2}'
  []
  [dp2_fun]
    type = ParsedFunction
    expression = '2*p_2'
    symbol_names = 'p_1 p_2'
    symbol_values = '${p1} ${p2}'
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = u
    function = u_fun
  []
  [dp1_ic]
    type = FunctionIC
    variable = dp1
    function = dp1_fun
  []
  [dp2_ic]
    type = FunctionIC
    variable = dp2
    function = dp2_fun
  []
[]

[VectorPostprocessors]
  [inner_product]
    type = ElementOptimizationReactionFunctionInnerProduct
    forward_variable = u
    variable = v
    function = p_fun
    execute_on = initial
  []
[]

[Postprocessors]
  [udp1v]
    type = VectorPostprocessorComponent
    vectorpostprocessor = inner_product
    vector_name = inner_product
    index = 0
  []
  [udp2v]
    type = VectorPostprocessorComponent
    vectorpostprocessor = inner_product
    vector_name = inner_product
    index = 1
  []
  [udp1v_exact]
    type = VariableInnerProduct
    variable = u
    second_variable = dp1
  []
  [udp2v_exact]
    type = VariableInnerProduct
    variable = u
    second_variable = dp2
  []
  [compare]
    type = ParsedPostprocessor
    function = 'abs(udp1v + udp1v_exact) + abs(udp2v + udp2v_exact)'
    pp_names = 'udp1v udp2v udp1v_exact udp2v_exact'
  []
[]

[UserObjects]
  [terminate]
    type = Terminator
    expression = 'compare > 1e-8'
    error_level = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
