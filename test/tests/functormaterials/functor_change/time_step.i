# Solves the IVP
#
#   dU/dt = -C U
#   U(0) = U0

C = 1.0
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

[Kernels]
  [time_derivative]
    type = TimeDerivative
    variable = U
  []
  [source]
    type = Reaction
    variable = U
    rate = ${C}
  []
[]

[FunctorMaterials]
  [mat]
    type = ADFunctorChangeFunctorMaterial
    functor = U
    change_over = time_step
    take_absolute_value = false
    prop_name = U_change
  []
[]

[Postprocessors]
  [U_avg]
    type = AverageNodalVariableValue
    variable = U
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [U_max_change]
    type = ElementExtremeFunctorValue
    functor = U_change
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient

  dt = 1.0
  num_steps = 5

  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
