!include problem.i

[Outputs]
  print_nonlinear_residuals = false
  print_linear_residuals = false
  csv = true
[]

[Postprocessors]
  [u]
    type = ElementAverageValue
    variable = u
  []
  [v]
    type = ElementAverageValue
    variable = v
  []
[]
