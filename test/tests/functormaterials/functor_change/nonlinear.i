# Solves the equation
#
#   U^2 = C
#   U(0) = U0

C = 25.0
U0 = 1.0

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [U]
    initial_condition = ${U0}
  []
[]

[FunctorMaterials]
  [residual_fmat]
    type = ADParsedFunctorMaterial
    expression = 'U^2 - C'
    functor_symbols = 'U C'
    functor_names = 'U ${C}'
    property_name = residual_prop
  []
[]

[Kernels]
  [residual]
    type = FunctorKernel
    variable = U
    functor = residual_prop
    functor_on_rhs = false
  []
[]

[FunctorMaterials]
  [mat]
    type = ADFunctorChangeFunctorMaterial
    functor = U
    change_over = nonlinear
    take_absolute_value = false
    prop_name = U_change
  []
[]

[Postprocessors]
  [U_avg]
    type = AverageNodalVariableValue
    variable = U
    execute_on = 'NONLINEAR_CONVERGENCE'
  []
  [U_max_change]
    type = ElementExtremeFunctorValue
    functor = U_change
    value_type = max
    execute_on = 'NONLINEAR_CONVERGENCE'
  []
[]

[Convergence]
  [nl_conv]
    type = IterationCountConvergence
    max_iterations = 2
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = none
  nonlinear_convergence = nl_conv
[]

[Outputs]
  [console]
    type = Console
    execute_postprocessors_on = 'NONLINEAR'
  []
  [csv]
    type = CSV
    file_base = nonlinear
    execute_on = 'NONLINEAR'
  []
[]
