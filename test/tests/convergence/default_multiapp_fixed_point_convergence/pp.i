!include parent.i

[Convergence]
  [fp_conv]
    custom_pp = small_value
    direct_pp_value = true
    custom_abs_tol = 1e-8
  []
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
