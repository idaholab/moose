!include base.i

[Kernels]
  [time_derivative]
    type = TimeDerivative
    variable = sol
  []
[]

[Postprocessors]
  [sol_nlit]
    type = AverageVariableChange
    variable = sol
    change_over = nonlinear_iteration
    norm = l1
    execute_on = ${pp_execute_on}
  []
  [sol_ts]
    type = AverageVariableChange
    variable = sol
    change_over = time_step
    norm = l1
    execute_on = ${pp_execute_on}
  []

  [aux_nlit]
    type = AverageVariableChange
    variable = aux
    change_over = nonlinear_iteration
    norm = l1
    execute_on = ${pp_execute_on}
  []
  [aux_ts]
    type = AverageVariableChange
    variable = aux
    change_over = time_step
    norm = l1
    execute_on = ${pp_execute_on}
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
[]
