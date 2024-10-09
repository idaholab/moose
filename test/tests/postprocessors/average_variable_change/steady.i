!include base.i

[Postprocessors]
  [sol_avg]
    type = ElementAverageValue
    variable = sol
    execute_on = ${pp_execute_on}
  []
  [sol_diff_l1]
    type = AverageVariableChange
    variable = sol
    change_over = nonlinear_iteration
    norm = l1
    execute_on = ${pp_execute_on}
  []
  [sol_diff_l2]
    type = AverageVariableChange
    variable = sol
    change_over = nonlinear_iteration
    norm = l2
    execute_on = ${pp_execute_on}
  []

  [aux_avg]
    type = ElementAverageValue
    variable = aux
    execute_on = ${pp_execute_on}
  []
  [aux_diff_l1]
    type = AverageVariableChange
    variable = aux
    change_over = nonlinear_iteration
    norm = l1
    execute_on = ${pp_execute_on}
  []
[]

[Executioner]
  type = Steady
[]
