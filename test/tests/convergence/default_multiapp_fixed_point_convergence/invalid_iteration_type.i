!include ../parent.i

[Convergence]
  [fp_conv]
    type = DefaultMultiAppFixedPointConvergence
  []
[]

[Executioner]
  nonlinear_convergence = fp_conv
[]
