[Variables]
  [T_fv]
    type = MooseVariableFVReal
  []
[]

[FunctorMaterials]
  [heat_flux_mat_elem]
    type = ADParsedFunctorMaterial
    expression = 'htc * (T - T_inf)'
    functor_symbols = 'T T_inf htc'
    functor_names = 'T_fv ${T_ambient} ${htc}'
    property_name = 'heat_flux_elem'
  []
  [source_mat_elem]
    type = ADParsedFunctorMaterial
    expression = '-B + A * (T - T_inf)^2'
    functor_symbols = 'A B T T_inf'
    functor_names = '${source_coef_A} ${source_coef_B} T_fv ${T_ambient}'
    property_name = 'source_elem'
  []
[]

[FVKernels]
  [T_fv_diff]
    type = FVDiffusion
    variable = T_fv
    coeff = ${k}
  []
  [T_fv_source]
    type = FVFunctorElementalKernel
    variable = T_fv
    functor_name = source_elem
  []
[]

[FVBCs]
  [left_bc_elem]
    type = FVDirichletBC
    variable = T_fv
    boundary = left
    value = ${T_ambient}
  []
  [right_bc_elem]
    type = FVFunctorNeumannBC
    variable = T_fv
    boundary = right
    functor = heat_flux_elem
    factor = -1
  []
[]

[Postprocessors]
  [fv_A_l1]
    type = DiscreteVariableResidualNorm
    variable = T_fv
    block = 'blockA'
    norm_type = l_1
    execute_on = 'FINAL'
  []
  [fv_B_l1]
    type = DiscreteVariableResidualNorm
    variable = T_fv
    block = 'blockB'
    norm_type = l_1
    execute_on = 'FINAL'
  []
[]
