!include parent.i

[Convergence]
  [it_conv]
    type = IterationCountConvergence
    max_iterations = 5
    converge_at_max_iterations = true
  []
[]

[Executioner]
  multiapp_fixed_point_convergence := it_conv
[]

[Outputs]
  file_base = other_convergence
[]
