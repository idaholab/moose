!include parent.i

[Convergence]
  [fp_conv]
    custom_abs_tol = 1e-8
  []
[]

[Executioner]
  custom_pp = small_value
[]

[Postprocessors]
  [small_value]
    type = ConstantPostprocessor
    value = 1e-10
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  file_base = pp
[]
