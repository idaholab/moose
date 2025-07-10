!include ../parent.i

[Convergence]
  [fp_conv]
    type = DefaultMultiAppFixedPointConvergence
    fixed_point_max_its = 15
    fixed_point_abs_tol = 1e50
  []
[]

[Outputs]
  file_base = residual
[]
