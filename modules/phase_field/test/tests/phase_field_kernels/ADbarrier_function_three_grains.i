# AD version of barrier_function_three_grains.i, shares its gold file
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]
[Variables]
  [gr0]
    [InitialCondition]
      type = FunctionIC
      function = '0.2+0.6*x'
    []
  []
  [gr1]
    [InitialCondition]
      type = FunctionIC
      function = '0.8-0.6*x*y'
    []
  []
  [gr2]
    [InitialCondition]
      type = FunctionIC
      function = '0.3+0.5*y'
    []
  []
[]
[Kernels]
  [gr0_time]
    type = ADTimeDerivative
    variable = gr0
  []
  [gr0_barrier]
    type = ADACBarrierFunction
    variable = gr0
    gamma = gamma
    v = 'gr1 gr2'
  []
  [gr1_time]
    type = ADTimeDerivative
    variable = gr1
  []
  [gr1_barrier]
    type = ADACBarrierFunction
    variable = gr1
    gamma = gamma
    v = 'gr0 gr2'
  []
  [gr2_time]
    type = ADTimeDerivative
    variable = gr2
  []
  [gr2_barrier]
    type = ADACBarrierFunction
    variable = gr2
    gamma = gamma
    v = 'gr0 gr1'
  []
[]
[Materials]
  [constants]
    type = ADGenericConstantMaterial
    prop_names = 'L gamma'
    prop_values = '1 1.5'
  []
  [mu]
    type = ADDerivativeParsedMaterial
    property_name = mu
    coupled_variables = 'gr0 gr1 gr2'
    expression = '16 * (gr0^2 * gr1^2 + gr1^2 * gr2^2 + gr0^2 * gr2^2 + 0.1)'
    derivative_order = 2
  []
[]
[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  num_steps = 2
  dt = 0.01
[]
[Outputs]
  exodus = true
  file_base = barrier_function_three_grains_out
[]
