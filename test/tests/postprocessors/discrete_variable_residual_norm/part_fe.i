[Variables]
  [T_fe]
  []
[]

[FunctorMaterials]
  [heat_flux_mat_nodal]
    type = ADParsedFunctorMaterial
    expression = 'htc * (T - T_inf)'
    functor_symbols = 'T T_inf htc'
    functor_names = 'T_fe ${T_ambient} ${htc}'
    property_name = 'heat_flux_nodal'
  []
  [source_mat_nodal]
    type = ADParsedFunctorMaterial
    expression = 'B - A * (T - T_inf)^2'
    functor_symbols = 'A B T T_inf'
    functor_names = '${source_coef_A} ${source_coef_B} T_fe ${T_ambient}'
    property_name = 'source_nodal'
  []
[]

[Kernels]
  [T_fe_diff]
    type = FunctionDiffusion
    variable = T_fe
    function = ${k}
  []
  [T_fe_source]
    type = FunctorKernel
    variable = T_fe
    functor = source_nodal
    functor_on_rhs = true
  []
[]

[BCs]
  [left_bc_nodal]
    type = DirichletBC
    variable = T_fe
    boundary = left
    value = ${T_ambient}
  []
  [right_bc_nodal]
    type = FunctorNeumannBC
    variable = T_fe
    boundary = right
    functor = heat_flux_nodal
    flux_is_inward = false
  []
[]

[Postprocessors]
  # l-1
  [fe_A_l1]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockA'
    norm_type = l_1
    execute_on = 'FINAL'
  []
  [fe_A_l1_nobias]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockA'
    norm_type = l_1
    correct_mesh_bias = true
    execute_on = 'FINAL'
  []
  [fe_B_l1]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockB'
    norm_type = l_1
    execute_on = 'FINAL'
  []

  # l-2
  [fe_A_l2]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockA'
    norm_type = l_2
    execute_on = 'FINAL'
  []
  [fe_A_l2_nobias]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockA'
    norm_type = l_2
    correct_mesh_bias = true
    execute_on = 'FINAL'
  []
  [fe_B_l2]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockB'
    norm_type = l_2
    execute_on = 'FINAL'
  []

  # l-infinity
  [fe_A_linf]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockA'
    norm_type = l_inf
    execute_on = 'FINAL'
  []
  [fe_A_linf_nobias]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockA'
    norm_type = l_inf
    correct_mesh_bias = true
    execute_on = 'FINAL'
  []
  [fe_B_linf]
    type = DiscreteVariableResidualNorm
    variable = T_fe
    block = 'blockB'
    norm_type = l_inf
    execute_on = 'FINAL'
  []
[]
