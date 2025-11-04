!include ../parent.i

[Convergence]
  [fp_conv]
    type = IterationCountConvergence
    max_iterations = 5
    converge_at_max_iterations = true
  []
[]

[Outputs]
  file_base = other_convergence
[]
